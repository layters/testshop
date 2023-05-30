#include "client.hpp"

#include "../../tools/logger.hpp"

#include <cstring> // memset
#include <cassert>

neroshop::Client::Client() : sockfd(-1), socket_type(SocketType::Socket_TCP) {
    create();
}
////////////////////
neroshop::Client::Client(int sockfd, struct sockaddr_in client_addr) {
    this->sockfd = sockfd;
    this->addr = client_addr;
}
////////////////////
neroshop::Client::~Client() {
    if(sockfd > 0) {
        shutdown();
        close();
        sockfd = -1;
    }
}
////////////////////
void neroshop::Client::create() {
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
    if(sockfd > 0) return; // socket must be -1 before a new one can be created (if socket is not null then it means it was never closed)
	sockfd = ::socket(AF_INET, (socket_type == SocketType::Socket_UDP) ? SOCK_DGRAM : SOCK_STREAM, 0);
	if(sockfd < 0) {
		neroshop::print("::socket: failed to create a socket", 1);
	}    
	// Set to non-blocking
	/*int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);*/
    
    // Set timeout for recv
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
    }
    #endif
}
////////////////////
bool neroshop::Client::connect(unsigned int port, std::string address) {
    // Clear the address structure
    memset(&this->addr, 0, sizeof(this->addr));
    
    // Set up hints for getaddrinfo
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC; // Use IPv4 or IPv6
    hints.ai_socktype = (socket_type == SocketType::Socket_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    
    // Get address info for the server
    struct addrinfo* result;
    int status = getaddrinfo(address.c_str(), std::to_string(port).c_str(), &hints, &result);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << std::endl;
        return false;
    }    

    for (struct addrinfo* p = result; p != NULL; p = p->ai_next) {
        if (::connect(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
            perror("connect failed");
            continue;
        }
        freeaddrinfo(result);
        return true;  // Return true immediately after a successful connection
    }

    freeaddrinfo(result);

    if (sockfd < 0) {
        std::cerr << "Could not connect to server" << std::endl;
        close();
        return false;
    }        
    return false;  // Return false if no successful connection was made
    //------------------------------------------------------------------
    /*#if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	struct hostent * host = gethostbyname(address.c_str()); // gethostbyname only supports IPv4 addresses, while getaddrinfo is a newer function that can handle both IPv4 and IPv6 addresses.
	if(host == nullptr) {
		std::cerr << "No host to connect to" << std::endl;
	}
	//struct sockaddr_in6 // IPv6	
	memset(&this->addr, 0, sizeof(struct sockaddr_in));
    this->addr.sin_family = AF_INET;
    memcpy(&this->addr.sin_addr.s_addr, host->h_addr, host->h_length);
    this->addr.sin_port = htons(port);
	// connect to a server	
	if(::connect(sockfd, (struct sockaddr *)(&this->addr), sizeof(this->addr)) < 0) {
		std::cerr << "Could not connect to server" << std::endl; // port is closed
		close(); // kill socket // https://linux.die.net/man/3/connect => Application Usage
		return false;
	}
	return true;    
    #endif
    return false;*/
}
////////////////////
void neroshop::Client::write(const std::string& text) {
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	ssize_t write_result = ::write(sockfd, text.c_str(), text.length());
	if(write_result < 0) { // -1 = error
		std::cerr << "Could not write to server" << std::endl;
	}    
    #endif
}
////////////////////
std::string neroshop::Client::read()
{
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
    char buffer[1024];
	memset(buffer, 0, 1024); // clear buffer (fills buffer with 0's) before reading into buffer//bzero(buffer, 1024); // bzero is deprecated
	ssize_t read_result = ::read(sockfd, buffer, 1023);
	if(read_result < 0) {
		std::cerr << "Could not read from server" << std::endl;	
	}
	return static_cast<std::string>(buffer);    
    #endif
    return "";
}
////////////////////
void neroshop::Client::send(const std::vector<uint8_t>& message) {
    assert(socket_type == SocketType::Socket_TCP && "Socket is not TCP");
    // ::send - instead of sending to a specific destination like sendto, send on SOCK_STREAM (TCP) socket sends the data to the connected socket, and returns the number of bytes sent.
    ssize_t sent_bytes = ::send(sockfd, message.data(), message.size(), 0);
    if (sent_bytes < 0) {
        perror("send");
    }
}
////////////////////
void neroshop::Client::send_to(const std::vector<uint8_t>& message, const struct sockaddr_in& dest_addr) {
    assert(socket_type == SocketType::Socket_UDP && "Socket is not UDP");
    // ::sendto()
    ssize_t sent_bytes = ::sendto(sockfd, message.data(), message.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
        perror("sendto");
    }    
}
////////////////////
void neroshop::Client::receive(std::vector<uint8_t>& message) {
    assert(socket_type == SocketType::Socket_TCP && "Socket is not TCP");

    const int BUFFER_SIZE = 4096;

    std::vector<uint8_t> buffer(BUFFER_SIZE);
    message.clear(); // clear the message vector before receiving new data
    // keep reading from the socket until the entire message is received
    size_t total_recv = 0;
    while (true) {
        ssize_t recv_bytes = ::recv(sockfd, buffer.data() + total_recv, BUFFER_SIZE - total_recv, 0);
        if (recv_bytes == -1) {
            perror("recv");
            return;
        }
        if (recv_bytes == 0) {
            break; // connection closed by server
        }
        total_recv += recv_bytes;
        if (total_recv == BUFFER_SIZE) {
            message.insert(message.end(), buffer.begin(), buffer.end());
            total_recv = 0;
        }
        if (total_recv >= message.size()) {
            break; // entire message received
        }
    }
    if (total_recv > 0) {
        message.insert(message.end(), buffer.begin(), buffer.begin() + total_recv);
    }
}
////////////////////
void neroshop::Client::receive_from(std::vector<uint8_t>& message, const struct sockaddr_in& addr) {
    assert(socket_type == SocketType::Socket_UDP && "Socket is not UDP");
    
    const int BUFFER_SIZE = 4096;
    
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    // addr must be constructed first before passing as arg
    socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
    ssize_t recv_bytes = ::recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addr_len);
    if (recv_bytes == -1) {
        perror("recvfrom");
        return;
    }
    if (recv_bytes > 0) {
        message.assign(buffer.data(), buffer.data() + recv_bytes);
    }
}
////////////////////	
void neroshop::Client::close() {
	::close(sockfd);
}
////////////////////
void neroshop::Client::shutdown() {
    ::shutdown(sockfd, SHUT_RDWR); // SHUT_RD, SHUT_WR, SHUT_RDWR
}
////////////////////
////////////////////
void neroshop::Client::disconnect() { // if only shutdown() is called, the client socket will still be alive which is why we must call close() as well
	shutdown();
	close();
}
////////////////////
bool neroshop::Client::reconnect(unsigned int port, std::string address) { // kill socket first before attempting to re-connect
    close();
    return connect(port, address);
}
////////////////////
neroshop::Client * neroshop::Client::get_main_client() {
    static neroshop::Client client_obj {};
    return &client_obj;
}
////////////////////
int neroshop::Client::get_socket() const {
    return sockfd;
}
////////////////////
////////////////////
bool neroshop::Client::is_connected() const { // https://stackoverflow.com/a/4142038 // can only work when close() is called
    return (sockfd != -1);
}
////////////////////
////////////////////
////////////////////
