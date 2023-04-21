#include <iostream>
#include <string>
#include <future>
#include <thread>
// neroshop
#include "../core/crypto/sha3.hpp"
#include "../core/protocol/p2p/node.hpp" // server.hpp included here (hopefully)
#include "../core/protocol/p2p/routing_table.hpp" // uncomment if using routing_table
#include "../core/protocol/rpc/bencode.hpp"
#include "../core/protocol/rpc/krpc.hpp"
#include "../core/protocol/transport/ip_address.hpp"
#include "../core/protocol/rpc/json_rpc.hpp"
#include "../core/database.hpp"
#include "../core/util/logger.hpp"
#include "../core/version.hpp"

#include <cxxopts.hpp>

#define NEROMON_TAG "\033[1;95m[neromon]:\033[0m "

// Daemon will handle database server requests from the client

using namespace neroshop;

Server server;

void close_server() {
    server.shutdown();
    std::cout << NEROMON_TAG "\033[1;91mdisconnected\033[0m" << std::endl;
}
////////////////////
///////////////////
// In case request is an HTTP request
// This function will extract the JSON payload from a HTTP request string that contains headers
/* This portion is excluded:
	   POST / HTTP/1.1
       Host: 127.0.0.1:57740
       User-Agent: curl/7.81.0
       Accept: -/-
       Content-Type: application/json
       Content-Length: 121
*/
std::string extract_json_payload(const std::string& request) {
    // Find the start of the JSON payload
    const std::string json_start = "\r\n\r\n";
    const auto json_pos = request.find(json_start);
    if (json_pos == std::string::npos) {
        // JSON payload not found
        return "";
    }
    // Extract the JSON payload
    const auto json_payload = request.substr(json_pos + json_start.size());
    return json_payload;
}
///////////////////
std::mutex clients_mutex;
std::mutex server_mutex;
///////////////////
void handle_requests() {
    // Lock the server mutex
    std::lock_guard<std::mutex> lock(server_mutex);

    // Read json-rpc request object from client
	std::string request_object = server.read();
	
	// Extract JSON payload from request
	const std::string json_payload = extract_json_payload(request_object);
	
	std::stringstream http_response;
	
	// Process JSON-RPC request
	std::string response_object = "";
	if(neroshop::rpc::is_json_rpc(json_payload)) {
	    response_object = neroshop::rpc::process(json_payload);
	    http_response << "HTTP/1.1 200 OK\r\n";
	} else {
        std::stringstream error_msg;
        error_msg << "{ \"error\": \"Invalid request format\" }";//error_msg << "{ \"jsonrpc\": \"2.0\", \"error\": { \"code\": -32600, \"message\": \"Invalid Request\", \"data\": \"Additional error information\" }, \"id\": null }";
        response_object = error_msg.str();
        http_response << "HTTP/1.1 400 Bad Request\r\n";
    }
	
	// Build HTTP response string
    http_response << "Content-Type: application/json\r\n";
    http_response << "Content-Length: " << response_object.length() << "\r\n";
    http_response << "\r\n";
    http_response << response_object;
	
	// Send HTTP response to client
	server.write(http_response.str());
}
///////////////////
void do_heartbeat()
{
    // Accept incoming connections and handle clients concurrently
    if(server.accept() != -1) {
        clients_mutex.lock();
        
        std::lock_guard<std::mutex> lock(server_mutex);
        
        std::thread request_thread(handle_requests);
        request_thread.detach();
        
        clients_mutex.unlock();
    }	
}
// For security purposes, we don't allow any arguments to be passed into the daemon
int main(int argc, char** argv)
{
    neroshop::Node dht_node("127.0.0.1", DEFAULT_PORT, true);//("0.0.0.0", DEFAULT_PORT);

    std::string daemon { "neromon" };
    std::string daemon_version { daemon + " v" + std::string(NEROSHOP_DAEMON_VERSION) };
    cxxopts::Options options(daemon, std::string(daemon_version));

    options.add_options()
        ("h,help", "Print usage")
        ("v,version", "Show version")
        ("b,bootstrap", "Run this node as a bootstrap node")////, cxxopts::value<std::string>())//("bl,bootstrap_lazy", "Run this node as a bootstrap node without specifying multiaddress")
        ("c,config", "Path to configuration file")
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
    if(result.count("bootstrap")) { 
        /*std::string multiaddress = result["bootstrap"].as<std::string>(); // ./neromon --bootstrap /ip4/12.144.256.32/tcp/57740 ; ./neromon --bootstrap "/ip6/[2001:0db8:85a3:0000:0000:8a2e:0370:7334]/tcp/4001" // Note that IPv6 addresses are enclosed in square brackets
        std::string ip_address; int port;
        try {
            auto address = neroshop::parse_multiaddress(multiaddress);
            ip_address = std::get<0>(address);
            port = std::get<1>(address);
            std::cout << "IP address + port: " << ip_address << ":" << port << std::endl;
        } catch (std::invalid_argument& e) {
            std::cerr << "Error: " << e.what() << " ( Format is: /ip4/<ip_address>/tcp/<port> )" << std::endl;
            exit(1);
        }
        
        assert(port == DEFAULT_TCP_PORT && "Port is not the default value.");*/
        
        std::cout << "Switching to bootstrap mode ...\n";
        dht_node.set_bootstrap(true);
        // TODO: bootstrap nodes will typically use both TCP and UDP
        // ALWAYS use public ip address for bootstrap nodes so that it is reachable by all nodes in the network, regardless of their location.
    }
    //-------------------------------------------------------

    std::cout << "******************************************************\n";
    std::cout << "Node ID: " << dht_node.get_id() << "\n";
    std::cout << "IP address: " << dht_node.get_ip_address() << "\n";
    std::cout << "Port number: " << dht_node.get_port() << "\n\n";
    std::cout << "******************************************************\n";
    // Join the DHT network
    dht_node.join(); // find_node message should be sent from here
    
    std::thread udp_peer_thread([&dht_node](){ dht_node.loop(); });
    udp_peer_thread.detach(); // detach threads so that they run independently
    

    //-------------------------------------------------------
    // Start TCP server
    std::atexit(close_server);
    
    int server_port = DEFAULT_TCP_PORT;
	if(server.bind(server_port)) {
	    std::cout << NEROMON_TAG "\033[1;97mTCP Server bound to port " + std::to_string(server_port) + "\033[0m\n";
	}
	server.listen(); // listens for any incoming connection
	
	const int SLEEP_INTERVAL = 1 * 1000; // must be specified in milliseconds
    // Enter daemon loop (main thread)
    while(true) {
        // Execute daemon heartbeat in a new thread
        std::thread heartbeat_thread(do_heartbeat);
        
        // Sleep for a period of time
        std::chrono::milliseconds sleep_duration(SLEEP_INTERVAL);
        std::this_thread::sleep_for(sleep_duration);

        // Join the heartbeat thread to wait for it to finish
        heartbeat_thread.join();        
    }
    //-------------------------------------------------------
	return 0;
}
