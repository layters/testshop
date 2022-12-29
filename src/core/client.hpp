#ifndef CLIENT_HPP_NEROSHOP
#define CLIENT_HPP_NEROSHOP

#include <uv.h>
//#include <raft.h>
#include <memory> // std::unique_ptr
#include <cstring> // memset
#if defined(__gnu_linux__)
#include <unistd.h> // ::close
#endif

#include "debug.hpp"
#include "server.hpp" // not sure if server should even be included in client ┐(´•_•`)┌

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

    //! \brief Requests from the server, a call to a functor binded to a name.
    //!
    //! This function calls already binded function from RPC Ccallable functions
    //!
    //! \param name The name of the functor.    
    // decltype (auto) detects the function's return type and allows us to return any return type 
    template <typename... Args>
    decltype (auto) call(const Server& server/*<= temporary*/, const std::string& name, Args&&... args) {   // todo: rename to request or nah? 
        // Print the arguments
        //(std::cout << ... << args);
        // Get number of arguments
        static const size_t arg_count = sizeof...(Args);
        // Check if function is not nullptr before calling it
        // Call function (this will work even if a function has zero args :D)
        return const_cast<Server&>(server).function_list[name](std::forward<Args>(args)...);
    }
    // For functions without a return value
    template <typename... Args>
    void call(const std::string& name, Args&&... args) {   // todo: rename to request or nah? 
    }    
private:
    #if defined(NEROSHOP_USE_LIBUV)
    uv_tcp_t handle;//uv_tcp_t client;
    //uv_udp_t handle; //(for receiving)
    //uv_udp_send_t request; //(for sending)
    #endif
    #if defined(__gnu_linux__)
	int socket;
	char buffer[256];    
	#endif
};
}
#endif
