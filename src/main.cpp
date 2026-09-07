#include <iostream>
#include <print>
#include <thread>

#include "storage/storage.hpp"
#include "server/server.hpp"
#include "client/client.hpp"

#define SERVER_PORT 9080

void testSend(Client& client, const std::string& payload) {
	client.send_to_server(payload);
	std::cout << "[request] " << client.receive() << "\n\n";
}

int main() {
    KeyValueStore kv{};
    Message::Parser parser;
    Message::Executor executor{kv};
    
    Server server{executor, parser, SERVER_PORT};
    std::thread server_thread([&] {
	    server.run();
	});
 
    {
        Client client{SERVER_PORT};

        testSend(client, "*2\r\n$3\r\nGET\r\n$4\r\ntest\r\n");              // GET test     (expecting 123)
        testSend(client, "*3\r\n$3\r\nSET\r\n$4\r\ntest\r\n$3\r\n321\r\n"); // SET test 321
        testSend(client, "*2\r\n$3\r\nGET\r\n$4\r\ntest\r\n");              // GET test     (expecting 321)

        auto v3 = kv.get("test");
        if (v3.has_value()) {
            std::println("value of 'test' = {}", v3.value());
        }

        // these calls should give errors
        testSend(client, "*2\r\n$3\r\nSET\r\n$4\r\ntest\r\n");              // SET test  
        testSend(client, "*1\r\n$3\r\nSET\r\n");                            // SET
        
        // malformed requests, none of these should return 200 OK
        testSend(client, "*2\r\n$999\r\nGET\r\n$4\r\ntest\r\n");
        testSend(client, "*2\r\n$3\r\nGET\r\n$999\r\ntest\r\n");
        testSend(client, "*1\r\n$3\r\nSET\r\n");
        testSend(client, "*4\r\n$3\r\nGET\r\n$4\r\ntest\r\n$3\r\nextra\r\n");

        // should return 200 OK; TCP may split single request across multiple receives
        client.send_to_server("*2\r\n$3\r\nGET\r\n$4\r\nt");
        client.send_to_server("est\r\n");
        std::cout << client.receive() << '\n';
    }

    server_thread.detach();

    return 0;
}