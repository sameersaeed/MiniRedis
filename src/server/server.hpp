#pragma once 

#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../concurrency/threadpool.hpp"
#include "../protocol/message.hpp"
#include "../storage/storage.hpp"
#include "../utils/utils.hpp"

namespace Message {
    class Executor;
    class Parser;
};

class ConnectionHandler {
public:
    ConnectionHandler(Message::Executor& executor, Message::Parser& parser, int client_fd) 
        : m_executor(executor), m_parser(parser), m_client_fd(client_fd)
    {}

    ~ConnectionHandler();

    void handle_command();
    bool receive_bytes();

	void send_response(const Command::Result& result);

private:
    Message::Executor& m_executor;
    Message::Parser& m_parser;

    int m_client_fd{-1};

    static constexpr size_t m_buffer_size = 1024;
    char m_buffer[m_buffer_size] = {0};

    std::string m_receive_buffer;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Server {
public:
    Server(Message::Executor& executor, Message::Parser& parser, size_t port,
        size_t thread_count = std::thread::hardware_concurrency());

    void run();

private:
    void bind_socket();
	void listen_for_connections();
    void accept_client();
    
    Message::Executor& m_executor;
    Message::Parser& m_parser;

    int m_server_fd{-1};
    size_t m_port;
    sockaddr_in m_addr;

    ThreadPool m_pool;
};