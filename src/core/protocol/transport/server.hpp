#ifndef SERVER_HPP_NEROSHOP
#define SERVER_HPP_NEROSHOP

#if defined(_WIN32) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
#include <winsock2.h> // core header for Winsock2
#include <ws2tcpip.h> // header for TCP/IP protocols
#include <iphlpapi.h> // header for IP helper functions
#endif

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

#include <cstdlib>
extern "C" {
#include <raft.h>
}

#include <unordered_map> // std::unordered_map
#include <functional> // std::function
#include <any> // std::any (C++17)
#include <stdexcept> // std::runtime_error
#include <cstring> // memset
#include <random> // std::random_device
#include <vector>

#include "client.hpp"

#define DEFAULT_TCP_PORT 57740 // Use ports between 49152-65535 that are not currently registered with IANA and are rarely used
#define DEFAULT_UDP_PORT 50881

#define DEFAULT_BACKLOG 511

namespace neroshop {

enum class SocketType {
    Socket_TCP = SOCK_STREAM,
    Socket_UDP = SOCK_DGRAM,
};

class Server {
private:
    SocketType socket_type;
public:
    Server();
    Server(SocketType socket_type);
    //Server(unsigned int port);
	~Server();
	bool bind(unsigned int port);
	bool listen(int backlog = DEFAULT_BACKLOG);
	bool accept();
	bool accept_all();
	void write(const std::string& message);
	std::string read();
	void close(); // closes socket
	void shutdown(); // shuts down entire connection, ending receiving and sending
	
	int get_socket() const;
	const neroshop::Client& get_client(int index) const;

private:
    int sockfd;
    struct sockaddr_in addr;
    std::vector<std::unique_ptr<Client>> clients;
    //raft_server_t* raft;
    friend class Node; // node can now access the server's functions
};
}
#endif
