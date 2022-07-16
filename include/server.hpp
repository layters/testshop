#ifndef SERVER_HPP_NEROSHOP
#define SERVER_HPP_NEROSHOP

#ifdef  _WIN32
#include "win32_header.hpp"
#endif
#ifdef __gnu_linux__
#include "linux_header.hpp"
#endif

#include <iostream>
#include <cstring> // memset
#include <thread>
#include <mutex>
#include <vector>
// libuv
#include <uv.h>
// raft
#include <raft.h>
// neroshop
#include "debug.hpp"

namespace neroshop {
class Server { // listens to a port and accepts a socket
public:
    Server();
	~Server();
	bool bind(unsigned int port);
	bool listen();
	bool accept();
	bool accept_all();
	void write(const std::string& text);
	std::string read();
	void close(); // closes socket
	void shutdown(); // shuts down entire connection, ending receiving and sending
	//void check_for_internet(); // server needs to make sure there is internet so that it can access database
	//InternetGetConnectedState();//win32
	// getters
	//std::string get_address() const;
	//unsigned int get_port() const;
#ifdef __gnu_linux__
	int get_socket() const;
	int get_client_socket(int index = 0) const;
#endif
private:
#ifdef _WIN32
	SOCKET socket, newsocket;
	char buffer[512];
	struct sockaddr_in server_addr, client_addr;
	socklen_t clilen;
#endif
#ifdef __gnu_linux__
	int socket, newsocket; // server-socket and client-socket //std::vector<int> newsockets;// temp
	char buffer[512]/*[256]*/; std::string buffer_new;
	struct sockaddr_in server_addr, client_addr;
	socklen_t clilen;
#endif	
	unsigned int port;
};
}
#endif
