#include "server.hpp"

#include <cassert>

#include "../../tools/logger.hpp"
#include "../../../neroshop_config.hpp"

namespace neroshop {

Server::Server() : sockfd(-1), socket_type(SocketType::Socket_TCP) {
	//#if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
		throw std::runtime_error("Failed to create socket.");
	}
}

Server::Server(SocketType socket_type) : sockfd(-1), socket_type(socket_type) {
    const auto protocol = (socket_type == SocketType::Socket_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    
    sockfd = ::socket(AF_INET, protocol, 0);
    if (sockfd < 0) {
        perror("socket");
        throw std::runtime_error("Failed to create socket.");
    }
}

Server::Server(const std::string& address, unsigned int port, SocketType socket_type) : sockfd(-1), socket_type(socket_type) {
    init_socket(address, port);
}
////////////////////
Server::~Server() {
    if(sockfd > 0) {
        shutdown();
        close();
        sockfd = -1;
    }
}
////////////////////
bool Server::bind(unsigned int port)
{
    memset(&this->addr, 0, sizeof(this->addr));
    this->addr.sin_port = htons(port);
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = INADDR_ANY; // sets the IP address in the sockaddr_in structure to the IP address of the current machine, which means that the server will be bound to all available network interfaces.

    if (::bind(sockfd, (struct sockaddr *) &this->addr, sizeof(this->addr)) == -1) {
        std::cerr << "Cannot bind socket" << std::endl;
        close();
        return false;
    }

	return true;
}
////////////////////
bool Server::bind(const std::string& address, unsigned int port) {
    // Configure socket address
    std::memset(&this->addr, 0, sizeof(this->addr));

    // Convert IP address or domain name to network byte order
    struct addrinfo hints, *result;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // Use IPv4 or IPv6
    hints.ai_socktype = (socket_type == SocketType::Socket_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int error = getaddrinfo(address.c_str(), NULL, &hints, &result);
    if (error != 0) {
        perror("getaddrinfo");
        std::cerr << "Failed to get address info: " << gai_strerror(error) << std::endl;
        return false;
    }

    // Find the first IPv4 or IPv6 address
    bool bound = false;
    for (struct addrinfo* rp = result; rp != nullptr; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            struct sockaddr_in* addr_in = (struct sockaddr_in*)rp->ai_addr;
            this->addr.sin_family = AF_INET;
            this->addr.sin_addr = addr_in->sin_addr;
            this->addr.sin_port = htons(port);
            bound = (::bind(sockfd, (struct sockaddr*)&this->addr, sizeof(this->addr)) == 0);
        }
        else if (rp->ai_family == AF_INET6) {
            struct sockaddr_in6* addr_in6 = (struct sockaddr_in6*)rp->ai_addr;
            this->addr6.sin6_family = AF_INET6;
            this->addr6.sin6_addr = addr_in6->sin6_addr;//in6addr_loopback; // Use the IPv6 loopback address
            this->addr6.sin6_port = htons(port);
            bound = (::bind(sockfd, (struct sockaddr*)&this->addr6, sizeof(this->addr6)) == 0);
        }
        if (bound) {
            break;
        }
    }

    freeaddrinfo(result);
    
    // Bind socket to address and port
    if (!bound) {
        perror("bind");
        std::cerr << "Failed to bind socket" << std::endl;
        return false;
    }
    
    return true;
}	
////////////////////	
bool Server::listen(int backlog)
{
    if (::listen(sockfd, backlog) == -1) {
        perror("listen");
        std::cerr << "Cannot listen for a connection" << std::endl;
        close();
        return false;
    }

    return true;
}
////////////////////
bool Server::accept() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(sockfd, (struct sockaddr*) &client_addr, &addr_len);

    if (client_fd == -1) {
        perror("accept");
        return false;
    }
    
    // Add the new client to the list of connected clients
    auto client = std::make_unique<Client>(client_fd, client_addr);
    clients.emplace_back(std::move(client));
    
    std::cout << "\033[0;37mReceived connection from " + std::string(inet_ntoa(client_addr.sin_addr)) + ":\033[0;36m" + std::to_string(ntohs(client_addr.sin_port)) + "\033[0m\n";
    return true;
}
////////////////////
void Server::write(const std::string& message) {
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	ssize_t write_result = ::write(clients.back()->sockfd, message.c_str(), message.length()/*buffer, strlen(buffer)*/);
    if(write_result < 0) { // -1 = error
        perror("write");
        std::cout << "Server unable to write to client" << std::endl;
    }    
    #endif
}
////////////////////
std::string Server::read() // receive data
{
#if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
    std::array<char, 1024> buffer{0}; // zero-initialize the buffer
    ssize_t read_result = ::read(clients.back()->sockfd, buffer.data(), buffer.size()-1);
    if (read_result < 0) {
        perror("socket read error:");
        shutdown();
        close();
    } else if (read_result == 0) {
        std::cerr << NEROSHOP_TAG "\033[0mClient orderly shut down the connection." << std::endl;
        //client.pop_back();
    } else {
        std::string result(buffer.data(), read_result);
        return result;
    }
#endif
    return "";
}
////////////////////
void Server::send(const std::vector<uint8_t>& message) {
    assert(socket_type == SocketType::Socket_TCP && "Socket is not TCP");

    if (message.empty()) {
        std::cerr << "Message is empty!" << std::endl;
        return;
    }
    
    const uint8_t* data = message.data();
    size_t size = message.size();
    
    // use a loop to repeatedly call send() until all the bytes are sent
    size_t total_sent = 0;
    while (total_sent < size) {
        int bytes_sent = ::send(clients.back()->sockfd, data + total_sent, size - total_sent, 0);
        if (bytes_sent == -1) {
            perror("send");
            break;
        }
        total_sent += bytes_sent;
    }
}

void Server::send_to(const std::vector<uint8_t>& message, const struct sockaddr_in& addr) {
    assert(socket_type == SocketType::Socket_UDP && "Socket is not UDP");

    if (message.empty()) {
        std::cerr << "Message is empty!" << std::endl;
        return;
    }
    
    const uint8_t* data = message.data();
    size_t size = message.size();

    if (addr.sin_family != AF_INET || addr.sin_port == 0) {
        std::cerr << "Invalid socket address!" << std::endl;
        return;
    }   
    // addr must be constructed first before passing as arg
    int bytes_sent = ::sendto(sockfd, data, size, 0, (struct sockaddr*)&addr, sizeof(addr));
    if (bytes_sent == -1) {
        perror("sendto");
    }
}
////////////////////
ssize_t Server::receive(std::vector<uint8_t>& message) { 
    assert(socket_type == SocketType::Socket_TCP && "Socket is not TCP");

    const int BUFFER_SIZE = 4096;
        
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    ssize_t recv_bytes = ::recv(clients.back()->sockfd, buffer.data(), BUFFER_SIZE, 0); // In the case of TCP, the server should receive from the client's sockfd, as this is the socket that is connected to the client and is used to communicate with the client.
    if (recv_bytes == -1) {
        perror("recv");
        return recv_bytes;
    }
    if (recv_bytes > 0) {
        message.assign(buffer.data(), buffer.data() + recv_bytes);
    }
    return recv_bytes;
}

ssize_t Server::receive_from(std::vector<uint8_t>& message, const struct sockaddr_in& addr) {
    assert(socket_type == SocketType::Socket_UDP && "Socket is not UDP");
    
    const int BUFFER_SIZE = 4096;
    
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    // addr must be constructed first before passing as arg
    socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
    ssize_t recv_bytes = ::recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addr_len); // A server should receive from its own sockfd in the case of UDP
    if (recv_bytes == -1) {
        perror("recvfrom");
        return recv_bytes;
    }
    if (recv_bytes > 0) {
        message.assign(buffer.data(), buffer.data() + recv_bytes);
    }
    return recv_bytes;
}
////////////////////
void Server::close() {
    ::close(sockfd);
    sockfd = -1;
    
    // Close all connected clients
    for (auto& client : clients) client->close();
    clients.clear();
}
////////////////////
void Server::shutdown() {
    ::shutdown(sockfd, SHUT_RDWR); // SHUT_RD, SHUT_WR, SHUT_RDWR
}
////////////////////
void Server::init_socket(const std::string& address, unsigned int port) {
	#if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	sockfd = ::socket(AF_INET, (socket_type == SocketType::Socket_UDP) ? SOCK_DGRAM : SOCK_STREAM, 0);
    if (sockfd < 0) {
		throw std::runtime_error("Failed to create socket.");
	}
	//----------------------------------
	/*This code sets a socket option to reuse the local address and port, which means that the same combination of IP address and port can be used by multiple sockets simultaneously, even if they are still in a TIME_WAIT state after the connection has been closed. This is useful for scenarios where you want to restart an application that uses a specific port quickly without waiting for the operating system to release the resources of the previous connection.

The setsockopt() function is used to set the socket option, and the SO_REUSEADDR and SO_REUSEPORT options are used to enable the reuse of local addresses and ports. The one variable is set to 1 to enable the options, and its address and size are passed to the setsockopt() function as arguments.*/
	// set socket options : SO_REUSEADDR (TCP:restart a closed/killed process on the same address so you can reuse address over and over again)
	// TO prevent the "Address already in use" error and allow reuse of local addresses and ports
	int opt = 1; // enable
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set socket option SO_REUSEADDR.");
    }
    
	if(!bind(address, port)) {
	    throw std::runtime_error("Failed to bound to port");
	}
	////std::cout << ((port == NEROSHOP_RPC_DEFAULT_PORT) ? "RPC server " : ((port == NEROSHOP_P2P_DEFAULT_PORT) ? "P2P server " : "IPC server ")) << "bound to port " << port << "\n";//std::cout << NEROMON_TAG "\033[1;97mServer " + "(TCP)" + " bound to port " + std::to_string(port) + "\033[0m\n";
	
	
	if(!listen()) {
	    throw std::runtime_error("Failed to listen for connection");
	}
	#endif    
}
////////////////////
////////////////////
int Server::get_socket() const {
    return sockfd;
}
////////////////////
const struct sockaddr_storage& Server::get_storage() const {
    return storage;
}
////////////////////
const neroshop::Client& Server::get_client(int index) const {
    if (index >= clients.size()) throw std::out_of_range("get_client: invalid or out of range index");
    return *(clients.at(index));
}

int Server::get_client_count() const {
    return clients.size();
}
////////////////////
////////////////////
void Server::set_nonblocking(bool nonblocking) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if(flags == -1) {
        perror("fcntl");
        throw std::runtime_error("set_nonblocking: Failed to get socket flags.");
        return;
    }
    
    if (nonblocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    if (fcntl(sockfd, F_SETFL, flags) == -1) {
        perror("fcntl");
        throw std::runtime_error("set_nonblocking: Failed to set socket to non-blocking mode.");
    }
}        
////////////////////
}
