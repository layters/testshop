#pragma once

#ifndef CLIENT_HPP_NEROSHOP
#define CLIENT_HPP_NEROSHOP

#if defined(_WIN32)
#include <winsock2.h> // core header for Winsock2
#include <ws2tcpip.h> // header for TCP/IP protocols
#include <iphlpapi.h> // header for IP helper functions
#endif

#if defined(__gnu_linux__)
#include <sys/socket.h> // for sockaddr_storage, AF_INET, AF_INET6
#include <netinet/in.h>
#include <arpa/inet.h> // for inet_pton
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
//#include <ifaddrs.h>
#include <errno.h>
#include <poll.h>
#endif

#if defined(NEROSHOP_USE_LIBZMQ)
#include <zmq.h>
#endif

#if defined(NEROSHOP_USE_LIBUV)
#include <uv.h>
#endif

#include <iostream>
#include <string>
#include <memory> // std::unique_ptr
#include <vector>

namespace neroshop {

enum class SocketType {
    Socket_TCP = 1,//SOCK_STREAM,
    Socket_UDP = 2,//SOCK_DGRAM,
};

class Client {
public:
    Client();
    Client(int sockfd, struct sockaddr_in client_addr);
	Client(std::string address, uint16_t port);
	~Client();
	void create(); // creates a new socket
	bool connect(uint16_t port, std::string address = "0.0.0.0");
	void write(const std::string& text);
	std::string read();
	void send(const std::vector<uint8_t>& message); // tcp
	void send_to(const std::vector<uint8_t>& message, const struct sockaddr_in& addr); // udp
    ssize_t receive(std::vector<uint8_t>& message); // tcp
    ssize_t receive_from(std::vector<uint8_t>& message, const struct sockaddr_in& addr); // udp
	// Interactions with the DHT node, which only exists on the client side via IPC server
	void put(const std::string& key, const std::string& value, std::vector<uint8_t>& response);
	void get(const std::string& key, std::vector<uint8_t>& response);
	void set(const std::string& key, const std::string& value, std::vector<uint8_t>& response);
	void remove(const std::string& key, std::vector<uint8_t>& response);
	void clear(std::vector<uint8_t>& response);
	void close(); // kills socket
	void shutdown(); // shuts down connection (disconnects from server)
    void disconnect(); // breaks connection to server then closes the client socket // combination of shutdown() and close()
    bool reconnect(uint16_t port, std::string address = "0.0.0.0"); // closes socket then connects again
    static Client * get_main_client();
    bool is_connected() const;
    
    int get_socket() const;
    int get_max_buffer_recv_size() const;

private:
	int sockfd;
	struct sockaddr_in addr;
	struct sockaddr_in6 addr6;
	SocketType socket_type;
	friend class Server;
};
}
#endif
