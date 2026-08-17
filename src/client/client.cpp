#include "client.hpp"

// sets up client and attempts to connect to server	    
Client::Client(size_t port) : m_port(port) {
    setup();
    connect_to_server(); 
}

// helper for sending request to server
void Client::send_to_server(const std::string& payload) const {
    size_t total_sent = 0;

    while (total_sent < payload.size()) {
        ssize_t bytes_sent = ::send(
            m_client_fd,
            payload.data() + total_sent,
            payload.size() - total_sent,
            0
        );

        if (bytes_sent <= 0) {
            return;
        }

        total_sent += bytes_sent;
    }
}	

// helper for receiving response from server
std::string Client::receive() {
    char buffer[buffer_size];

    ssize_t bytes_read = ::read(m_client_fd, buffer, buffer_size);

    if (bytes_read <= 0) {
        return {};
    }

    return std::string(buffer, bytes_read);
}

void Client::setup() {
    // setup file descriptor
    if ((m_client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket creation error\n";
        return;
    }

    // configure server address and port
    m_server_addr = { 
        .sin_family=AF_INET, 
        .sin_port=htons(m_port)
    };

    // convert server IP address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &m_server_addr.sin_addr) <= 0) {
        std::cerr << "invalid address or address isn't supported\n";
        return;
    }
    std::cout << "[client] setup successful\n\n";
}

void Client::connect_to_server() {
    if (::connect(
        m_client_fd, 
        reinterpret_cast<struct sockaddr*>(&m_server_addr), 
        sizeof(m_server_addr)
    ) < 0) {
        std::cerr << "connection to server failed from client\n";
        return;
    }
}