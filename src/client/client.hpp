#pragma once 

#include <iostream>

#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>

class Client {
public:
	explicit Client(size_t port);
    ~Client();
    
	void send_to_server(const std::string& payload) const;
	std::string receive();

private:
	void setup();
    void connect_to_server();
    
    int m_client_fd{-1};

    size_t m_port;
    sockaddr_in m_server_addr;

    static constexpr size_t buffer_size = 1024;
};
