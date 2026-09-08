#include <chrono>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include "client/client.hpp"
#include "server/server.hpp"
#include "storage/storage.hpp"

// spin up one server and use it for all tests
class IntegrationTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        s_store = std::make_unique<KeyValueStore>();
        s_parser = std::make_unique<Message::Parser>();
        s_executor = std::make_unique<Message::Executor>(*s_store);
        s_server = std::make_unique<Server>(*s_executor, *s_parser, TEST_PORT, 4);

        s_server_thread = std::thread([] { s_server->run(); });
        s_server_thread.detach();

        // give the server a sec to actually start listening
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    static constexpr size_t TEST_PORT = 9099;

    static inline std::unique_ptr<KeyValueStore> s_store;
    static inline std::unique_ptr<Message::Parser> s_parser;
    static inline std::unique_ptr<Message::Executor> s_executor;
    static inline std::unique_ptr<Server> s_server;
    static inline std::thread s_server_thread;
};

// basic SET then GET over a real socket connection
TEST_F(IntegrationTest, SetThenGetOverRealSocket) {
    Client client{TEST_PORT};

    client.send_to_server("*3\r\n$3\r\nSET\r\n$1\r\nx\r\n$1\r\ny\r\n");
    std::string set_response = client.receive();
    EXPECT_NE(set_response.find("200 OK"), std::string::npos);

    client.send_to_server("*2\r\n$3\r\nGET\r\n$1\r\nx\r\n");
    std::string get_response = client.receive();
    EXPECT_NE(get_response.find("200 OK"), std::string::npos);
    EXPECT_NE(get_response.find("y"), std::string::npos);
}

// two clients hitting the server at the same time shouldnt mix up each others data
TEST_F(IntegrationTest, MultipleClientsGetIndependentResponses) {
    Client client_a{TEST_PORT};
    Client client_b{TEST_PORT};

    client_a.send_to_server("*3\r\n$3\r\nSET\r\n$4\r\nkeya\r\n$1\r\na\r\n");
    client_b.send_to_server("*3\r\n$3\r\nSET\r\n$4\r\nkeyb\r\n$1\r\nb\r\n");

    std::string resp_a = client_a.receive();
    std::string resp_b = client_b.receive();

    EXPECT_NE(resp_a.find("200 OK"), std::string::npos);
    EXPECT_NE(resp_b.find("200 OK"), std::string::npos);

    EXPECT_EQ(s_store->get("keya"), "a");
    EXPECT_EQ(s_store->get("keyb"), "b");
}