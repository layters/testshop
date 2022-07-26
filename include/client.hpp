#ifndef CLIENT_HPP_NEROSHOP
#define CLIENT_HPP_NEROSHOP

#include <uv.h>
//#include <raft.h>
#include <memory> // std::unique_ptr

#include "debug.hpp"

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
    static Client * get_main_client();
    bool is_connected() const;
private:
    uv_tcp_t handle;//uv_tcp_t client;
    //uv_udp_t handle; //(for receiving)
    //uv_udp_send_t request; //(for sending)
    static std::unique_ptr<Client> singleton;
};
}
#endif
