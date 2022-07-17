#ifndef SERVER_HPP_NEROSHOP
#define SERVER_HPP_NEROSHOP

#include <uv.h>
#include <raft.h>

#include "debug.hpp"
#include "database.hpp"

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
private:
};
}
#endif
