#ifndef CLIENT_HPP_NEROSHOP
#define CLIENT_HPP_NEROSHOP

#if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
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
	Client(std::string address, unsigned int port);
	~Client();
	void create(); // creates a new socket
	bool connect(unsigned int port, std::string address = "0.0.0.0");
	void write(const std::string& text);
	std::string read();
	void send();
	void send(const std::vector<uint8_t>& packed); // tcp
	void send_to(const std::vector<uint8_t>& packed, const struct sockaddr_in& addr); // udp
    std::string receive(); // tcp
    std::string receive_from(); // udp
	void close(); // kills socket
	void shutdown(); // shuts down connection (disconnects from server)
    void disconnect(); // breaks connection to server then closes the client socket // combination of shutdown() and close()
    bool reconnect(unsigned int port, std::string address = "0.0.0.0"); // closes socket then connects again
    static Client * get_main_client();
    bool is_connected() const;
    
    int get_socket() const;

private:
	int sockfd;
	struct sockaddr_in addr;
	SocketType socket_type;
	friend class Server;
	int epoll_fd;
};
}
#endif
