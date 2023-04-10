#include <iostream>
// neroshop
#include "../core/server.hpp"
#include "../core/database.hpp"
#include "../core/protocol/rpc/json_rpc.hpp"
#include "../core/util/logger.hpp"
#include "../core/version.hpp"

#include <cxxopts.hpp>

#define NEROMON_TAG "\033[1;95m[neromon]:\033[0m "

// Daemon will handle database server requests from the client

using namespace neroshop;

Server * server;

void close_server() {
    server->shutdown();
    delete server; // calls destructor which calls closesocket()
    server = nullptr;
    std::cout << NEROMON_TAG "\033[1;91mdisconnected\033[0m" << std::endl;
}
////////////////////
///////////////////
void do_heartbeat()
{
    // accept multiple connections - I noticed that new clients are only accepted when the primary client writes to the server
    if(server->accept() != -1) // accepts any incoming connection
    {
	    //std::cout << "server's client_socket: " << server->get_client_socket() << std::endl;// returns 5
	    //std::thread new_client(client); // create a new client thread each time it accepts
	    //new_client.join();
	    ////std::cout << NEROMON_TAG "\033[1;32mconnected\033[0m\n";
    } //else exit(0);
    // Read json-rpc request object from client
	std::string request_object = server->read();
	// Response to client with a json-rpc response object
	server->write(neroshop::rpc::process(request_object));
}
// For security purposes, we don't allow any arguments to be passed into the daemon
int main(int argc, char** argv)
{
    std::string daemon { "neromon" };
    std::string daemon_version { daemon + " v" + std::string(NEROSHOP_DAEMON_VERSION) };
    cxxopts::Options options(daemon, std::string(daemon_version));

    options.add_options()
        ("h,help", "Print usage")
        ("v,version", "Show version")
        ("datadir", "Path to storage of nershop data")
    ;
    
    auto result = options.parse(argc, argv);
    
    if(result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    if(result.count("version")) {
        std::cout << daemon << " version " << std::string(NEROSHOP_DAEMON_VERSION) << std::endl;
        exit(0);
    }        
    //-------------------------------------------------------
    // Start server
    std::atexit(close_server);
    
    server = new Server();
    
    int server_port = 40441;//1234;//(std::stoi(port));
	if(server->bind(server_port)) {
	    std::cout << NEROMON_TAG "\033[1;97mbound to port " + std::to_string(server_port) + "\033[0m\n";
	}
	server->listen(); // listens for any incoming connection
	
	const int SLEEP_INTERVAL = 1;
  // Enter daemon loop
    while(true) {
        // Execute daemon heartbeat
        do_heartbeat();
        // Sleep for a period of time
        #ifdef _WIN32
        Sleep(SLEEP_INTERVAL);
        #else
        sleep(SLEEP_INTERVAL);
        #endif
    }	
	return 0;
}
