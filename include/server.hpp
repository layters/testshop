#ifndef SERVER_HPP_NEROSHOP
#define SERVER_HPP_NEROSHOP

#include <uv.h>
#include <raft.h>
#include <unordered_map> // std::unordered_map
#include <functional> // std::function
//#if defined(__cplusplus) && (__cplusplus >= 201703L)
#include <any> // std::any (C++17)
//#endif
#include <stdexcept> // std::runtime_error

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
	// todo: figure out how to bind strings (requests) to functions (responses) with different argument types and counts
    
    //! \brief Binds a functor to a name so it becomes callable via RPC.
    //!
    //! This function template accepts a wide range of callables. The arguments
    //! and return types of these callables should be serializable by msgpack.
    //! `bind` effectively generates a suitable, light-weight compile-time
    //! wrapper for the functor.
    //!
    //! \param name The name of the functor.
    //! \param func The functor to bind.
    //! \tparam F The type of the functor.
    // bind function should work with lambdas too
    template <typename F> void bind(std::string const &name, F functor) {
        if(!functor) throw std::runtime_error("bind invalid function");
        function_list[name] = functor;
    }

    //! \brief Unbinds a functor binded to a name.
    //!
    //! This function removes already binded function from RPC Ccallable functions
    //!
    //! \param name The name of the functor.
    void unbind(std::string const &name) {
        function_list[name] = nullptr;
    }    
private:
    uv_tcp_t * handle_tcp;
    uv_udp_t * handle_udp;
    // callbacks
    void on_new_connection(uv_stream_t *server, int status);
    // functors
    //#if defined(__cplusplus) && (__cplusplus < 201703L)
    //std::unordered_map<std::string, std::function<void()>/*adaptor_type*/> function_list;
    //#endif
    //#if defined(__cplusplus) && (__cplusplus >= 201703L)
    std::unordered_map<std::string, std::any> function_list;
    //#endif
    // references:
    // https://github.com/rpclib/rpclib/blob/master/include/rpc/dispatcher.h
    // https://github.com/rpclib/rpclib/blob/master/include/rpc/server.h
};
}
#endif
