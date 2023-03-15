#ifndef SERVER_HPP_NEROSHOP
#define SERVER_HPP_NEROSHOP

#if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
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

#include "database.hpp"
#include "debug.hpp"

namespace neroshop {
class Server {
public:
    Server();
    //Server(unsigned int port);
	~Server();
	bool bind(unsigned int port);
	bool listen();
	bool accept();
	bool accept_all();
	void write(const std::string& message);
	std::string read();
	void close(); // closes socket
	void shutdown(); // shuts down entire connection, ending receiving and sending

private:
    #if defined(__gnu_linux__) && defined(NEROSHOP_USE_SYSTEM_SOCKETS)
    int socket;
    char buffer[1024];
    int client_socket;
    #endif    
    raft_server_t* raft;
    // functors
    //#if defined(__cplusplus) && (__cplusplus < 201703L)
    //std::unordered_map<std::string, std::function<void()>/*adaptor_type*/> functions;
    //#endif
    //#if defined(__cplusplus) && (__cplusplus >= 201703L)
    std::unordered_map<std::string, std::any> functions;
    //#endif
    // ??
    /*template< class R, class... Args >
    std::unordered_map<std::string, std::function<R(Args...)>> functions;*/
    // references:
    // https://github.com/rpclib/rpclib/blob/master/include/rpc/dispatcher.h
    // https://github.com/rpclib/rpclib/blob/master/include/rpc/server.h
};
}
#endif
