#ifndef CLIENT_HPP_NEROSHOP
#define CLIENT_HPP_NEROSHOP

#ifdef  _WIN32
#include "win32_header.hpp"
#endif
#ifdef __gnu_linux__
#include "linux_header.hpp"
#endif
// libuv
#include <uv.h>
// raft
#include <raft.h>
// neroshop
#include "debug.hpp"

#include <iostream>
#include <cstring>
#include <thread>
#include <mutex>

namespace neroshop {
class Client {
public:
    Client();
	Client(std::string address, unsigned int port);
	~Client();
	void create(); // creates a new socket
	bool connect(unsigned int port, std::string address = "0.0.0.0");
	void write(const std::string& text);
	std::string read();
	void close(); // kills socket
	void shutdown(); // shuts down connection (disconnects from server)
    void disconnect(); // breaks connection to server then closes the client socket // combination of shutdown() and close()
    bool reconnect(unsigned int port, std::string address = "0.0.0.0"); // closes socket then connects again
#ifdef _WIN32
	SOCKET get_socket() const;
#endif
#ifdef __gnu_linux__
	int get_socket() const;
#endif
    static Client * get_main_client();
    bool is_connected() const;
private:
#ifdef _WIN32
	SOCKET socket;
	char buffer[512];	
	struct sockaddr_in socket_addr;	
    struct hostent * host;
#endif
#ifdef __gnu_linux__
	int socket;
	char buffer[256];
	// struct sockaddr // universal endpoint type
	struct sockaddr_in socket_addr;	// IPv4 
	// struct sockaddr_in6 // IPv6
	// sockaddr_un // ??
	// 4 types of sockets in POSIX API: TCP, UDP, UNIX, and (optionally) RAW
    struct hostent * host;
#endif
    static std::unique_ptr<Client> client_obj;
    bool test_socket();// const; // temporary
};
}
#endif
/*
void server()
{
    // server
	Server * server = new Server();
	server->bind(1234);
	server->listen();
	server->accept();
	server->write("Welcome to Server0"); // write to client once
	while(1) // read from client (while program is running)
		std::cout << server->read() << std::endl; 
}
void client()
{
	// client
    Client * client = new Client();
	client->connect(1234, "localhost");
	std::cout << client->read() << std::endl; // read from server once
	while (1)  // write to server (while program is running)
	{
		std::string text;
		std::cin >> text; 
		client->write(text);
	}
}
*/
