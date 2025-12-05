#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <shared_mutex>
#include <csignal> // std::signal, SIGINT
// neroshop
#include "../core/crypto/sha3.hpp"
#include "../core/protocol/p2p/node.hpp"
#include "../core/protocol/p2p/routing_table.hpp"
#include "../core/network/sam_client.hpp"
#include "../core/network/tor_manager.hpp"
#include "../core/protocol/transport/server.hpp"
#include "../core/protocol/rpc/json_rpc.hpp"
#include "../core/protocol/rpc/msgpack.hpp"
#include "../core/protocol/rpc/protobuf.hpp"
#include "../core/database/database.hpp"
#include "../core/tools/logger.hpp"
#include "../core/version.hpp"
#include "../core/tools/filesystem.hpp"

#include <cxxopts.hpp>

using namespace neroshop;

//-----------------------------------------------------------------------------

std::mutex server_mutex;
std::condition_variable server_cv;

std::atomic<bool> running(true);

void signal_handler(int signum) {
    std::cout << "\nSIGINT caught. Exiting...\n";
    running = false;
    server_cv.notify_all(); // Notify any threads waiting
}

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
    server.set_nonblocking(true);
    server.listen();
    
    while (running) {
        // Accept incoming connections and handle clients concurrently
        if(server.accept()) {            
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
                    response_object = neroshop::rpc::json_process(json_payload);
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
        
        if (!running) {
            // Stop accepting new connections and exit the loop
            break;
        }        
    }
    // Close the server socket before exiting
    neroshop::log_info("[rpc_server] Closing RPC server");
    server.shutdown();
    server.close();
}

//-----------------------------------------------------------------------------

void ipc_server(Node& node) {
    // Prevent seed node from being accepted by IPC server 
    // since its only meant to act as an initial contact point for new nodes joining the network
    if (node.is_hardcoded()) {
        neroshop::log_info("Seed node is not allowed to use the local IPC server. Please start another daemon instance to interact with the GUI or CLI");
        return;
    }
    
    Server server("127.0.0.1", NEROSHOP_IPC_DEFAULT_PORT);
    server.listen();
    
    while (node.is_running()) {
        if(server.accept()) { // ONLY accepts a single client
            while (node.is_running()) { // Keep accepting messages from the client until running is false
                std::vector<uint8_t> request;
                // wait for incoming message from client
                int recv_size = server.receive(request);
                if (recv_size == -1) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // No data available right now, continue to the next loop iteration
                        continue;
                    }
                }
                if (recv_size == 0) {
                    // Republish data before exiting (or in case of client app crashes)
                    node.republish_once();
                    // Send signal to stop both DHT and IPC servers
                    raise(SIGINT);
                    // Connection closed by client, break out of loop (so we don't process invalid bytes)
                    break;
                }
                std::vector<uint8_t> response;
                {
                    // Perform both read and write operations on the node object (Node should be thread-safe internally)
                    #if defined(NEROSHOP_USE_MSGPACK)
                    response = neroshop::rpc::msgpack_process(request, node, true);
                    #elif defined(NEROSHOP_USE_PROTOBUF)
                    response = neroshop::rpc::protobuf_process(request, node, true);
                    #else
                    #error "Serialization backend not defined"
                    #endif
                }      
                // Send response to client
                server.send(response);
            }
        }
    }  
    // Close the server socket before exiting // TODO: implement SIGINT (Ctrl+C)
    neroshop::log_info("[ipc_server] Closing local IPC server");
    server.close();
}

//-----------------------------------------------------------------------------

void dht_server(Node& node) {
    // If using Tor, wait until Tor is ready before joining
    if (node.get_network_type() == NetworkType::Tor) {
        auto tor_manager = node.get_tor_manager();
        while (tor_manager && !tor_manager->is_tor_ready()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    std::cout << "******************************************************\n";
    if(node.get_network_type() == NetworkType::I2P) {
        std::cout << "SAM Session ID: " << node.get_sam_client()->get_nickname() << "\n";
    }
    std::cout << "Node ID: " << node.get_id() << "\n";
    std::cout << "Address: " << node.get_address() << "\n";
    std::cout << "Port number: " << node.get_port() << "\n\n";
    std::cout << "******************************************************\n";
    // Start the DHT node's main loop in a separate thread
    std::thread run_thread([&]() {
        node.run();
    });

    // Join the DHT network
    if (!node.is_hardcoded()) {
        node.join(); // A seed node cannot join the network
    }
    
    if(node.is_hardcoded()) {
        node.rebuild_routing_table();
    }

    run_thread.join(); // Wait for the run thread to finish
    
    neroshop::log_info("[dht_server] Closing DHT server");
}

//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
    std::signal(SIGINT, signal_handler); // Handles Ctrl+C

    std::string daemon { "neroshopd" };
    std::string daemon_version { daemon + " v" + std::string(NEROSHOP_DAEMON_VERSION) };
    cxxopts::Options options(daemon, std::string(daemon_version));

    options.add_options()
        ("h,help", "Print usage")
        ("v,version", "Show version")
        ("rpc,enable-rpc", "Enables the RPC daemon server")
        ////("conf,config", "Set path to configuration file", cxxopts::value<std::string>()->default_value("/some_path"))
        ("public,public-node", "Make this node publicly accessible")
        ("network,network-type", "Set anonymous overlay network [i2p | tor]", cxxopts::value<std::string>())
        ("socks-port", "Set SocksPort in torrc file", cxxopts::value<unsigned int>())
    ;
    
    options.parse_positional({"seed-node"}); // allows for multiple args
    
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
    
    neroshop::NetworkType network_type = neroshop::NetworkType::Tor; // default
    if(result.count("network")) {
        // If anonymous overlay network is invalid, throw error
        std::string network = result["network"].as<std::string>();
        std::string network_lower = string_tools::lower(network);
        if(network_lower != "i2p" && network_lower != "tor") {
             throw std::invalid_argument("invalid overlay network");
        }
        
        // Set the network_type based on the command line args
        if(network_lower == "i2p") {
            network_type = neroshop::NetworkType::I2P;
        } else if(network_lower == "tor") {
            network_type = neroshop::NetworkType::Tor;
        }
        
        std::cout << "\033[1;90mSelected overlay network: " << network_lower << "\033[0m\n";
    }
    
    uint16_t socks_port = 9052;//9050; // <- 9050 may likely be already in use by another app
    if(result.count("socks-port")) {
        if(network_type != neroshop::NetworkType::Tor) {
            std::cerr << "Error: --socks-port option is only valid with --network tor\n";
            std::cout << "Usage: " << argv[0] << " --network tor --socks-port 9052\n";
            exit(0);
        }
        
        socks_port = result["socks-port"].as<unsigned int>();
    }
    
    //-------------------------------------------------------
    // create "datastore" folder within "~/.config/neroshop/" path (to prevent sqlite3_open: out of memory error)
    std::string data_dir = neroshop::get_default_database_path();
    if(!neroshop::filesystem::is_directory(data_dir)) {
        if(!neroshop::filesystem::make_directory(data_dir)) {
            throw std::runtime_error("Failed to create neroshop data dir");
        }
    }
    
    db::Sqlite3 * database = neroshop::get_database();
    if(!database->table_exists("mappings")) { 
        database->execute("CREATE VIRTUAL TABLE mappings USING fts5(search_term, key, content, tokenize = \"porter unicode61 remove_diacritics 1 tokenchars '-_:'\");"); // 0=uses accent characters (diacritics) like é; default is 1
    }
    //-------------------------------------------------------
    // Start TorManager on a separate thread
    auto tor_manager = std::make_shared<neroshop::TorManager>(socks_port);
    try {
        std::thread tor_thread([&tor_manager]() {
            tor_manager->start_tor();

            while (!tor_manager->is_tor_ready()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                printf("Tor bootstrap progress: %d%%\n", tor_manager->get_bootstrap_progress());
            }
            printf("Tor is ready. Onion address: %s\n", tor_manager->get_onion_address().c_str());
        });

        // Make sure to join or manage tor_thread lifecycle properly
        tor_thread.detach(); // or join as appropriate

    } catch (const std::exception& e) {
        std::cerr << "Failed to start TorManager: " << e.what() << std::endl;
        return 1;
    }
    //-------------------------------------------------------
    neroshop::Node node(network_type, tor_manager);
    
    std::thread ipc_thread([&node]() { ipc_server(node); }); // For IPC communication between the local GUI client and the local daemon server
    std::thread dht_thread([&node]() { dht_server(node); }); // DHT communication for peer discovery and data storage
    std::thread rpc_thread;  // Declare the thread object // RPC communication for processing requests from outside clients (disabled by default)
    
    if(result.count("rpc")) {
        neroshop::log_info("RPC enabled, listening on port {} using JSON-RPC 2.0", NEROSHOP_RPC_DEFAULT_PORT);
        rpc_thread = std::thread(rpc_server, std::cref(ip_address));  // Initialize the thread object 
    }
    
    // Wait for all threads to finish
    if (rpc_thread.joinable()) {
        rpc_thread.join();
    }
    ipc_thread.join();
    dht_thread.join();
    
    return 0;
}
