#include "server.hpp"

ConnectionHandler::~ConnectionHandler() { 
    if (m_client_fd != -1)
        ::close(m_client_fd); 
}

bool ConnectionHandler::receive_bytes() {
    ssize_t bytes_read = ::read(m_client_fd, m_buffer, m_buffer_size);
    if (bytes_read <= 0) {
        return false;
    }

    m_receive_buffer.append(m_buffer, bytes_read);
 
    // ouputting request command, next line after will be response code
    for (ssize_t i = 0; i < bytes_read; ++i) {
        if (m_buffer[i] == '\r')
            std::cout << "\\r";
        else if (m_buffer[i] == '\n')
            std::cout << "\\n";
        else
            std::cout << m_buffer[i];
    }

    std::cout << "'\n";
 
    return true;
}

void ConnectionHandler::handle_command() {
    while (true) {
        auto result = m_parser.parse(m_receive_buffer);

        switch (result.status) {
            case Message::Parser::Status::Incomplete: // need to read more
                break;

            case Message::Parser::Status::Invalid: {
                // discard malformed data to avoid repeated reparsing
                m_receive_buffer.clear();

                ConnectionHandler::send_response(Command::Result{
                    .status = Utils::Response::BadRequest,
                    .value = std::nullopt
                });

                break;
            }

            case Message::Parser::Status::Complete: {
                // remove command from buffer
                m_receive_buffer.erase(0, result.request->bytes_consumed);
                
                auto response = m_executor.execute(*result.request);
                send_response(response);

                break;
            }
        }

        if (result.status == Message::Parser::Status::Incomplete)
            break;
    }
}

void ConnectionHandler::send_response(const Command::Result& result) {
    std::string response{ "[response] " };

    switch (result.status) {
        case Utils::Response::OK:
            response += "200 OK";
            break;

        case Utils::Response::BadRequest:
            response += "400 Bad Request";
            break;

        case Utils::Response::NotFound:
            response += "404 Not Found";
            break;
    }

    response += "\r\n";

    if (result.value.has_value()) {
        response += *result.value;
        response += "\r\n";
    }

    size_t total_sent = 0;

    while (total_sent < response.size()) {
        ssize_t bytes_sent = ::send(
            m_client_fd,
            response.data() + total_sent,
            response.size() - total_sent,
            0
        );

        if (bytes_sent <= 0)
            return;

        total_sent += bytes_sent;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Server::Server(Message::Executor& executor, Message::Parser& parser, size_t port, size_t thread_count) 
    : m_executor(executor), m_parser(parser), m_port(port), m_pool(thread_count > 0 ? thread_count : 1) {

    // create socket
    if ((m_server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server socket creation failed\n");
        exit(EXIT_FAILURE);
    }

    // set socket options
    int opt = 1;
    if (setsockopt(m_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(m_server_fd);
        exit(EXIT_FAILURE);
    }

    // configure server address and port
    m_addr = {};
    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons(m_port);

    // bind server to port and listen for incoming connections
    bind_socket();
    listen_for_connections();
}


void Server::run() {
    while (true) {
        accept_client();
    }
}

// bind will take m_addr, user can only configure port so we can use it internally
void Server::bind_socket() {
    if (bind(m_server_fd, reinterpret_cast<struct sockaddr*>(&m_addr), sizeof(m_addr)) < 0) {
        perror("bind failed");
        close(m_server_fd);
        exit(EXIT_FAILURE);
    }
}

void Server::listen_for_connections() {
    if (::listen(m_server_fd, 1) < 0) {
        perror("listen failed");
        close(m_server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "[server] listening on port " << m_port << "\n\n";
}

// wait for client connection, we wont allow receive() to happen til a client connects
void Server::accept_client() {
    socklen_t addr_len = sizeof(m_addr);

    int client_fd = accept(
        m_server_fd, 
        reinterpret_cast<struct sockaddr*>(&m_addr), 
        &addr_len
    );

    if (client_fd < 0) {
        perror("accept failed");
        return;
    }	

    m_pool.enqueue([this, client_fd] {
        ConnectionHandler handler(m_executor, m_parser, client_fd);
        
        while (handler.receive_bytes()) {
            handler.handle_command();
        }
    });
}