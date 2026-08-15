#pragma once 

#include <iostream>
#include <string>
#include <string_view>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../utils/utils.hpp"
#include "../storage/storage.hpp"
#include "../protocol/message.hpp"

class Server {
public:
    Server(Message::Executor& executor, Message::Parser& parser, size_t port);
    void run();

private:
    void bind_socket();
	void listen_for_connections();
    void accept_client();
	void send_response(const Command::Result& result);

	Message::Executor& m_executor;
    Message::Parser& m_parser;

	static constexpr size_t buffer_size = 1024;
    size_t m_port;
	
    int m_server_fd{-1};
    int m_client_fd{-1};

    sockaddr_in m_addr;

    std::string m_receive_buffer;
};