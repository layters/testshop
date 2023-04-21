#include "server.hpp"

#include "../../database.hpp"
#include "../../util/logger.hpp"

neroshop::Server::Server() : sockfd(-1), socket_type(SocketType::Socket_TCP) {
	#if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
		throw std::runtime_error("Failed to create socket.");
	}
	/*This code sets a socket option to reuse the local address and port, which means that the same combination of IP address and port can be used by multiple sockets simultaneously, even if they are still in a TIME_WAIT state after the connection has been closed. This is useful for scenarios where you want to restart an application that uses a specific port quickly without waiting for the operating system to release the resources of the previous connection.

The setsockopt() function is used to set the socket option, and the SO_REUSEADDR and SO_REUSEPORT options are used to enable the reuse of local addresses and ports. The one variable is set to 1 to enable the options, and its address and size are passed to the setsockopt() function as arguments.*/
	// set socket options : SO_REUSEADDR (TCP:restart a closed/killed process on the same address so you can reuse address over and over again)
	int opt = 1; // enable
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set socket option SO_REUSEADDR.");
    }
	#endif    
}
////////////////////
neroshop::Server::Server(SocketType socket_type) : socket_type(socket_type) {
    const auto protocol = (socket_type == SocketType::Socket_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    
    sockfd = ::socket(AF_INET, protocol, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket.");
    }
    //std::cout << "Created a " << ((protocol == SOCK_DGRAM) ? "UDP socket" : "TCP socket") << "\n";
}
////////////////////
neroshop::Server::~Server() {
    if(sockfd > 0) {
        close();
        sockfd = -1;
    }
}
////////////////////
bool neroshop::Server::bind(unsigned int port)
{
    memset(&this->addr, 0, sizeof(this->addr));
    this->addr.sin_port = htons(port);
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = INADDR_ANY;

    if (::bind(sockfd, (struct sockaddr *) &this->addr, sizeof(this->addr)) == -1) {
        std::cerr << "Cannot bind socket" << std::endl;
        close();
        return false;
    }

	return true;
}
////////////////////	
bool neroshop::Server::listen(int backlog)
{
    if (::listen(sockfd, backlog) == -1) {
        std::cerr << "Cannot listen for a connection" << std::endl;
        close();
        return false;
    }

    return true;
}
////////////////////
bool neroshop::Server::accept() {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(sockfd, (struct sockaddr*) &client_addr, &addr_len);

    if (client_fd == -1) {
        return false;
    }
    
    // Add the new client to the list of connected clients
    auto client = std::make_unique<Client>(client_fd, client_addr);
    clients.emplace_back(std::move(client));
    
    std::cout << "\033[0;37mReceived connection from " + std::string(inet_ntoa(client_addr.sin_addr)) + ":\033[0;36m" + std::to_string(ntohs(client_addr.sin_port)) + "\033[0m\n";
    
    return true;
}
////////////////////
void neroshop::Server::write(const std::string& message) {
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	ssize_t write_result = ::write(clients.back()->sockfd, message.c_str(), message.length()/*buffer, strlen(buffer)*/);
    if(write_result < 0) { // -1 = error
        std::cout << "Server unable to write to client" << std::endl;
    }    
    #endif
}
////////////////////
std::string neroshop::Server::read() // receive data
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
void neroshop::Server::close() {
    ::close(sockfd);
    sockfd = -1;
    
    // Close all connected clients
    for (auto& client : clients) client->close();
    clients.clear();
}
////////////////////
void neroshop::Server::shutdown() {
    ::shutdown(sockfd, SHUT_RDWR); // SHUT_RD, SHUT_WR, SHUT_RDWR
}
////////////////////
////////////////////
int neroshop::Server::get_socket() const {
    return sockfd;
}
////////////////////
const neroshop::Client& neroshop::Server::get_client(int index) const {
    if (index >= clients.size()) throw std::out_of_range("get_client: invalid or out of range index");
    return *(clients.at(index));
}
////////////////////
////////////////////
