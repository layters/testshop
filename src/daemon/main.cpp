#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <shared_mutex>
// neroshop
#include "../core/crypto/sha3.hpp"
#include "../core/protocol/p2p/node.hpp" // server.hpp included here (hopefully)
#include "../core/protocol/p2p/routing_table.hpp" // uncomment if using routing_table
#include "../core/protocol/transport/ip_address.hpp"
#include "../core/protocol/rpc/json_rpc.hpp"
#include "../core/protocol/messages/msgpack.hpp"
#include "../core/database/database.hpp"
#include "../core/tools/logger.hpp"
#include "../core/version.hpp"

#include <cxxopts.hpp>

#define NEROMON_TAG "\033[1;95m[neromon]:\033[0m "

// Daemon will handle database server requests from the client

using namespace neroshop;

std::mutex clients_mutex;
std::mutex server_mutex;
std::shared_mutex node_mtx; // Shared mutex to protect access to the Node object
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------

void rpc_server(const std::string& address) {
    Server server(address, NEROSHOP_RPC_DEFAULT_PORT);
    
    while (true) {
        // Accept incoming connections and handle clients concurrently
        if(server.accept() != -1) {
            std::thread request_thread([&]() {
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
                    response_object = neroshop::rpc::json::process(json_payload);
                    http_response << "HTTP/1.1 200 OK\r\n";
                } else {
                    std::stringstream error_msg;
                    nlohmann::json error_obj;
                    error_obj["jsonrpc"] = "2.0";
                    error_obj["error"] = {};
                    error_obj["error"]["code"] = -32600;
                    error_obj["error"]["message"] = "Invalid Request";//error_obj["error"]["data"] = "Additional error information"; // may be ommited
                    error_obj["id"] = nullptr;
                    error_msg << error_obj.dump();
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
            });

            request_thread.detach();
        }
    }
    // Close the server socket before exiting
    server.close();
}

//-----------------------------------------------------------------------------

void ipc_server(Node& node) {
    Server server("127.0.0.1", NEROSHOP_IPC_DEFAULT_PORT);
    
    while (true) {
        if(server.accept() != -1) {  // ONLY accepts a single client
            while (true) {
                std::vector<uint8_t> request;
                // wait for incoming message from client
                int recv_size = server.receive(request);
                if (recv_size == 0) {
                    // Connection closed by client, break out of loop
                    break;
                }
                std::vector<uint8_t> response;
                {
                    //std::unique_lock<std::shared_mutex> lock(node_mtx);//std::shared_lock<std::shared_mutex> lock(node_mtx);  // Acquire shared lock before accessing the node object // Locking the node_mtx causes the IPC server to not respond to the client requests for some reason :/
                    
                    // process JSON request and generate response
                    response = neroshop::msgpack::process(request, node, true);
                }
                // The shared_lock is destroyed and the lock is released here          
                // send response to client
                server.send(response);
            }
        } 
    }  
    // Close the server socket before exiting // TODO: implement SIGINT (Ctrl+C)
    server.close();
}

//-----------------------------------------------------------------------------

// still needs a lot of work. I have no idea what I'm doing :/
void dht_server(Node& node) {
    /*Server server("127.0.0.1", NEROSHOP_P2P_DEFAULT_PORT, SocketType::Socket_UDP);
    server.set_nonblocking(true);*/
    // Acquire lock before accessing the node object
    ////std::lock_guard<std::shared_mutex> lock(node_mtx);
    std::cout << "******************************************************\n";
    std::cout << "Node ID: " << node.get_id() << "\n";
    std::cout << "IP address: " << node.get_ip_address() << /*" (" << node.get_public_ip_address() << ")*/"\n";
    std::cout << "Port number: " << node.get_port() << "\n\n";
    std::cout << "******************************************************\n";
    // Start the DHT node's main loop in a separate thread
    std::thread run_thread([&]() {
        //std::unique_lock<std::shared_mutex> lock(node_mtx);  // Acquire exclusive lock before accessing the node object
        node.run();
    });

    // Join the DHT network
    if (!node.is_bootstrap_node()) {
        //std::unique_lock<std::shared_mutex> lock(node_mtx);//std::shared_lock<std::shared_mutex> lock(node_mtx);  // Acquire shared lock before accessing the node object
        node.join(); // A bootstrap node cannot join the network
    }

    run_thread.join(); // Wait for the run thread to finish
    // The unique_lock is destroyed and the lock is released here
}

//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    std::string daemon { "neromon" };
    std::string daemon_version { daemon + " v" + std::string(NEROSHOP_DAEMON_VERSION) };
    cxxopts::Options options(daemon, std::string(daemon_version));

    options.add_options()
        ("h,help", "Print usage")
        ("v,version", "Show version")
        ("b,bootstrap", "Run this node as a bootstrap node")//("bl,bootstrap-lazy", "Run this node as a bootstrap node without specifying multiaddress")//("c,config", "Path to configuration file", cxxopts::value<std::string>())
        ("rpc,enable-rpc", "Enables the RPC daemon server")
        ("public,public-node", "Make your daemon into a public node")
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

    if(result.count("config")) {
        std::string config_path = result["config"].as<std::string>();
        if(!config_path.empty()) {}
    }
    
    std::string ip_address = NEROSHOP_LOOPBACK_ADDRESS;
    if(result.count("public")) {
        ip_address = NEROSHOP_ANY_ADDRESS;
    }
    //-------------------------------------------------------
    neroshop::Node node(ip_address, NEROSHOP_P2P_DEFAULT_PORT, true);
    
    if(result.count("bootstrap")) {   
        std::cout << "Switching to bootstrap mode ...\n";
        node.set_bootstrap(true);
        #ifdef NEROSHOP_RELEASE
        assert(ip_address == NEROSHOP_ANY_ADDRESS && "Bootstrap node is not public");
        #endif
        // ALWAYS use address "0.0.0.0" for bootstrap nodes so that it is reachable by all nodes in the network, regardless of their location.
    }
    //-------------------------------------------------------
    std::thread ipc_thread([&node]() { ipc_server(node); });//(ipc_server, std::ref(node));//  // For IPC communication between the local GUI client and the local daemon server
    std::thread dht_thread([&node]() { dht_server(node); });//(dht_server, std::ref(node));// // DHT communication for peer discovery and data storage
    std::thread rpc_thread;  // Declare the thread object // RPC communication for processing requests from outside clients (disabled by default)
    
    if(result.count("rpc")) {
        std::cout << "RPC enabled\n";
        rpc_thread = std::thread(rpc_server, std::cref(ip_address));  // Initialize the thread object 
    }
    
    // Wait for all threads to finish
    if (rpc_thread.joinable()) {
        rpc_thread.join();
    }
    ipc_thread.join();
    dht_thread.join(); // Uses a ton of resources due to UDP socket being non-blocking + when calling run() :/

	return 0;
}
