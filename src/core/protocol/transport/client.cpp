#include "client.hpp"

#include "../../tools/logger.hpp"
#include "../../version.hpp" // NEROSHOP_DHT_VERSION

#include <cstring> // memset
#include <cassert>

#include <nlohmann/json.hpp>

namespace neroshop {

Client::Client() : sockfd(-1), socket_type(SocketType::Socket_TCP) {
    create();
}
////////////////////
Client::Client(int sockfd, struct sockaddr_in client_addr) {
    this->sockfd = sockfd;
    this->addr = client_addr;
}
////////////////////
Client::~Client() {
    if(sockfd > 0) {
        shutdown();
        close();
        sockfd = -1;
    }
}
////////////////////
void Client::create() {
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
    if(sockfd > 0) return; // socket must be -1 before a new one can be created (if socket is not null then it means it was never closed)
	sockfd = ::socket(AF_INET, (socket_type == SocketType::Socket_UDP) ? SOCK_DGRAM : SOCK_STREAM, 0);
	if(sockfd < 0) {
		neroshop::log_error("::socket: failed to create a socket");
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
bool Client::connect(unsigned int port, std::string address) {
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
void Client::write(const std::string& text) {
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
	ssize_t write_result = ::write(sockfd, text.c_str(), text.length());
	if(write_result < 0) { // -1 = error
		std::cerr << "Could not write to server" << std::endl;
	}    
    #endif
}
////////////////////
std::string Client::read()
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
void Client::send(const std::vector<uint8_t>& message) {
    assert(socket_type == SocketType::Socket_TCP && "Socket is not TCP");
    // ::send - instead of sending to a specific destination like sendto, send on SOCK_STREAM (TCP) socket sends the data to the connected socket, and returns the number of bytes sent.
    ssize_t sent_bytes = ::send(sockfd, message.data(), message.size(), 0);
    if (sent_bytes < 0) {
        perror("send");
    }
}
////////////////////
void Client::send_to(const std::vector<uint8_t>& message, const struct sockaddr_in& dest_addr) {
    assert(socket_type == SocketType::Socket_UDP && "Socket is not UDP");
    // ::sendto()
    ssize_t sent_bytes = ::sendto(sockfd, message.data(), message.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent_bytes < 0) {
        perror("sendto");
    }    
}
////////////////////
ssize_t Client::receive(std::vector<uint8_t>& message) {
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
            return recv_bytes;
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
    return total_recv;
}
////////////////////
ssize_t Client::receive_from(std::vector<uint8_t>& message, const struct sockaddr_in& addr) {
    assert(socket_type == SocketType::Socket_UDP && "Socket is not UDP");
    
    const int BUFFER_SIZE = 4096;
    
    std::vector<uint8_t> buffer(BUFFER_SIZE);
    // addr must be constructed first before passing as arg
    socklen_t addr_len = static_cast<socklen_t>(sizeof(addr));
    ssize_t recv_bytes = ::recvfrom(sockfd, buffer.data(), BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addr_len);
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
void Client::put(const std::string& key, const std::string& value, std::string& reply) {
    // Send put - no id or tid required for IPC client requests. The DHT server will deal with that
    nlohmann::json args_obj = { {"key", key}, {"value", value} };
    nlohmann::json query_object = { {"version", std::string(NEROSHOP_DHT_VERSION)}, {"query", "put"}, {"args", args_obj}, {"tid", nullptr} };
    std::vector<uint8_t> packed_data = nlohmann::json::to_msgpack(query_object);
    send(packed_data);
    // Receive response
    try {
        std::vector<uint8_t> response;
        receive(response);
        try {
            nlohmann::json response_object = nlohmann::json::from_msgpack(response);
            reply = response_object.dump();//return response_object.dump();
        } catch (const nlohmann::detail::parse_error& e) {
            std::cerr << "Failed to parse server response: " << e.what() << std::endl;
        }
    } catch (const nlohmann::detail::parse_error& e) {
        std::cerr << "An error occurred: " << "Node was disconnected" << std::endl;
    }
}

void Client::get(const std::string& key, std::string& reply) {
    // Send get - no id or tid required for IPC client requests. The DHT server will deal with that
    nlohmann::json args_obj = { {"key", key} };
    nlohmann::json query_object = { {"version", std::string(NEROSHOP_DHT_VERSION)}, {"query", "get"}, {"args", args_obj}, {"tid", nullptr} };
    std::vector<uint8_t> packed_data = nlohmann::json::to_msgpack(query_object);
    send(packed_data);
    // Receive response
    try {
        std::vector<uint8_t> response;
        receive(response);
        try {
            nlohmann::json response_object = nlohmann::json::from_msgpack(response);
            reply = response_object.dump();//return response_object.dump();
        } catch (const nlohmann::detail::parse_error& e) {
            std::cerr << "Failed to parse server response: " << e.what() << std::endl;
        }
    } catch (const nlohmann::detail::parse_error& e) {
        std::cerr << "An error occurred: " << "Node was disconnected" << std::endl;
    }    
}

void Client::set(const std::string& key, const std::string& value, std::string& reply) {
    // Send set - no id or tid required for IPC client requests. The DHT server will deal with that
    nlohmann::json args_obj = { {"key", key}, {"value", value} };
    nlohmann::json query_object = { {"version", std::string(NEROSHOP_DHT_VERSION)}, {"query", "set"}, {"args", args_obj}, {"tid", nullptr} };
    std::vector<uint8_t> packed_data = nlohmann::json::to_msgpack(query_object);
    send(packed_data);
    // Receive response
    try {
        std::vector<uint8_t> response;
        receive(response);
        try {
            nlohmann::json response_object = nlohmann::json::from_msgpack(response);
            reply = response_object.dump();//return response_object.dump();
        } catch (const nlohmann::detail::parse_error& e) {
            std::cerr << "Failed to parse server response: " << e.what() << std::endl;
        }
    } catch (const nlohmann::detail::parse_error& e) {
        std::cerr << "An error occurred: " << "Node was disconnected" << std::endl;
    }
}

void Client::remove(const std::string& key, std::string& reply) {
    // Send remove - no id or tid required for IPC client requests. The DHT server will deal with that
    nlohmann::json args_obj = { {"key", key} };
    nlohmann::json query_object = { {"version", std::string(NEROSHOP_DHT_VERSION)}, {"query", "remove"}, {"args", args_obj}, {"tid", nullptr} };
    std::vector<uint8_t> packed_data = nlohmann::json::to_msgpack(query_object);
    send(packed_data);
    // Receive response
    try {
        std::vector<uint8_t> response;
        receive(response);
        try {
            nlohmann::json response_object = nlohmann::json::from_msgpack(response);
            reply = response_object.dump();//return response_object.dump();
        } catch (const nlohmann::detail::parse_error& e) {
            std::cerr << "Failed to parse server response: " << e.what() << std::endl;
        }
    } catch (const nlohmann::detail::parse_error& e) {
        std::cerr << "An error occurred: " << "Node was disconnected" << std::endl;
    }
}

void Client::clear(std::string& reply) {
    nlohmann::json args_obj = nlohmann::json::object();
    nlohmann::json query_object = { {"version", std::string(NEROSHOP_DHT_VERSION)}, {"query", "clear"}, {"args", args_obj}, {"tid", nullptr} };
    std::vector<uint8_t> packed_data = nlohmann::json::to_msgpack(query_object);
    send(packed_data);
    // Receive response
    try {
        std::vector<uint8_t> response;
        receive(response);
        try {
            nlohmann::json response_object = nlohmann::json::from_msgpack(response);
            reply = response_object.dump();//return response_object.dump();
        } catch (const nlohmann::detail::parse_error& e) {
            std::cerr << "Failed to parse server response: " << e.what() << std::endl;
        }
    } catch (const nlohmann::detail::parse_error& e) {
        std::cerr << "An error occurred: " << "Node was disconnected" << std::endl;
    }
}
////////////////////	
void Client::close() {
	::close(sockfd);
}
////////////////////
void Client::shutdown() {
    ::shutdown(sockfd, SHUT_RDWR); // SHUT_RD, SHUT_WR, SHUT_RDWR
}
////////////////////
////////////////////
void Client::disconnect() { // if only shutdown() is called, the client socket will still be alive which is why we must call close() as well
	shutdown();
	close();
}
////////////////////
bool Client::reconnect(unsigned int port, std::string address) { // kill socket first before attempting to re-connect
    close();
    return connect(port, address);
}
////////////////////
neroshop::Client * Client::get_main_client() {
    static neroshop::Client client_obj {};
    return &client_obj;
}
////////////////////
int Client::get_socket() const {
    return sockfd;
}
////////////////////
int Client::get_max_buffer_recv_size() const {
    int max_buffer_size;
    socklen_t bufferSizeLen = sizeof(max_buffer_size);
    if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &max_buffer_size, &bufferSizeLen) == 0) {
        // `max_buffer_size` contains the maximum receive buffer size
        std::cout << "Maximum receive buffer size: " << max_buffer_size << " bytes" << std::endl;
    } else {
        perror("getsockopt"); // Failed to retrieve the maximum receive buffer size
        return -1; // Return an error value indicating failure
    }
    return max_buffer_size;
}
////////////////////
////////////////////
bool Client::is_connected() const { // https://stackoverflow.com/a/4142038 // can only work when close() is called
    return (sockfd != -1);
}
////////////////////
////////////////////
////////////////////
}
