#include "node.hpp"

#include "kademlia.hpp"
#include "../../crypto/sha3.hpp"
#include "../../crypto/rsa.hpp"
#include "routing_table.hpp"
#include "../transport/ip_address.hpp"
#include "../messages/msgpack.hpp"
#include "../../version.hpp"
#include "../../tools/base64.hpp"
#include "mapper.hpp"
#include "../../tools/timestamp.hpp"
#include "../../database/database.hpp"

#include <nlohmann/json.hpp>

#include <cstring> // memset
#include <future>
#include <iomanip> // std::set*
#include <cassert>
#include <thread>
#include <unordered_set>

namespace neroshop_crypto = neroshop::crypto;
namespace neroshop_timestamp = neroshop::timestamp;

neroshop::Node::Node(const std::string& address, int port, bool local) : sockfd(-1), bootstrap(false), check_counter(0) { 
    // Convert URL to IP (in case it happens to be a url)
    std::string ip_address = neroshop::ip::resolve(address);
    // Generate a random node ID - use public ip address for uniqueness
    public_ip_address = (local) ? get_public_ip_address() : ip_address;
    id = generate_node_id(public_ip_address, port);
    //---------------------------------------------------------------------------
    memset(&storage, 0, sizeof(storage));
    if(is_ipv4(ip_address)) storage.ss_family = AF_INET;
    if(is_ipv6(ip_address)) storage.ss_family = AF_INET6;
    //---------------------------------------------------------------------------
    // If this node is a local node that you own
    if(local == true) {
        // Create a UDP socket
        sockfd = socket(storage.ss_family, SOCK_DGRAM, 0);
        if(sockfd < 0) {
            perror("socket");
            throw std::runtime_error("::socket failed");
        }

        // set a timeout of TIMEOUT_VALUE seconds for recvfrom
        struct timeval tv;
        tv.tv_sec = NEROSHOP_DHT_QUERY_RECV_TIMEOUT;  // timeout in seconds
        tv.tv_usec = 0; // timeout in microseconds
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
            std::cerr << "Error setting socket options" << std::endl;
            close(sockfd);
            exit(0);
        }
        
        // Make sockfd non-blocking
        int flags = fcntl(sockfd, F_GETFL, 0);
        if(flags == -1) {
            perror("fcntl");
        }
        if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl");
        }

        // To avoid "bind" error and allow multiple nodes to run on the same IP address, we can use different port numbers for each local node we create
        // Bind to port
        int port_dynamic = port; // initial port number
        const int MAX_PORT_NUM = 65535; // port cannot go past this number
        bool use_ephemeral_port = false;

        while(true) {        
            if(storage.ss_family == AF_INET) {
                sockin = {0};//memset(&sockin, 0, sizeof(sockin)); // both approaches are valid, but memset is more reliable and portable
                sockin.sin_family = storage.ss_family;
                sockin.sin_port = htons(port_dynamic);//htons(std::stoi(std::to_string(port_dynamic))); // the second approach may be useful in cases where the port number is being manipulated as a string or integer elsewhere in the code
                if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin.sin_addr) <= 0) { //sockin.sin_addr.s_addr = htonl(INADDR_ANY) - binds to any available network interface // inet_addr(ip_address.c_str()) - binds to a specific ip // recommended to use inet_pton() over inet_addr() when working with networking in modern systems.
                    perror("inet_pton");
                }
            }
            if(storage.ss_family == AF_INET6) {
                memset(&sockin6, 0, sizeof(sockin6));
                sockin6.sin6_family = storage.ss_family;
                sockin6.sin6_port = htons(port_dynamic);
                if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin6.sin6_addr) <= 0) { 
                    perror("inet_pton");
                }
            }
            
            if(bind(sockfd, (storage.ss_family == AF_INET6) ? (struct sockaddr *)&sockin6 : (struct sockaddr *)&sockin, (storage.ss_family == AF_INET6) ? sizeof(sockin6) : sizeof(sockin)) == 0) {
                // Update node ID in case the port ever changes
                id = generate_node_id(public_ip_address, port_dynamic);
                std::cout << "DHT node bound to port " << port_dynamic << std::endl;
                break;
            }
            std::cout << "Port " << port_dynamic << " already in use." << std::endl;

            use_ephemeral_port = true;
            if(use_ephemeral_port) {
                // Bind to the ephemeral port number
                sockin.sin_port = htons(0);
                if(bind(sockfd, (storage.ss_family == AF_INET6) ? (struct sockaddr *)&sockin6 : (struct sockaddr *)&sockin, (storage.ss_family == AF_INET6) ? sizeof(sockin6) : sizeof(sockin)) == 0) {
                    // Get the actual port number used by the socket
                    struct sockaddr_in local_addr;
                    socklen_t local_addr_len = sizeof(local_addr);
                    getsockname(sockfd, (struct sockaddr*)&local_addr, &local_addr_len);
                    port_dynamic = ntohs(local_addr.sin_port);
                    // Generate new node ID with the ephemeral port number
                    id = generate_node_id(public_ip_address, port_dynamic);
                    std::cout << "DHT node bound to ephemeral port " << port_dynamic << std::endl;
                    break;
                }
            }
            // Use the next available port number
            port_dynamic++;
            if (port_dynamic > MAX_PORT_NUM) {
                throw std::runtime_error("Unable to bind to any available port.");
            }
        }

        // Node is now bound to a unique port number
    }
    //---------------------------------------------------------------------------
    // If this is an external node that you do not own
    if(local == false) {
        sockfd = socket(storage.ss_family, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket");
        }
        // This socket is not meant to be bound to any port. It will only be used to retrieve the IP address and port number
    
        if(storage.ss_family == AF_INET) {
            memset(&sockin, 0, sizeof(sockin));
            sockin.sin_family = storage.ss_family;
            sockin.sin_port = htons(port);
            if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin.sin_addr) <= 0) { 
                perror("inet_pton");
            }
        }
        if(storage.ss_family == AF_INET6) {
            memset(&sockin6, 0, sizeof(sockin6));
            sockin6.sin6_family = storage.ss_family;
            sockin6.sin6_port = htons(port);
            if(inet_pton(storage.ss_family, ip_address.c_str(), &sockin6.sin6_addr) <= 0) { 
                perror("inet_pton");
            }
        }
        // Set socket options, such as timeout or buffer size, if needed
    }
    
    // Create the routing table with an empty vector of nodes
    if(!routing_table.get()) {
        routing_table = std::make_unique<RoutingTable>(std::vector<Node*>{});
        routing_table->my_node_id = this->id;
    } 
       
    // Initialize key mapper
    if(!mapper.get()) {
        mapper = std::make_unique<Mapper>();
    }
}

neroshop::Node::Node(Node&& other) noexcept
    : id(std::move(other.id)),
      version(std::move(other.version)),
      data(std::move(other.data)),
      info_hash_peers(std::move(other.info_hash_peers)),
      //server(std::move(other.server)),
      sockfd(other.sockfd),
      sockin(std::move(other.sockin)),
      sockin6(std::move(other.sockin6)),
      storage(std::move(other.storage)),
      routing_table(std::move(other.routing_table)),
      public_ip_address(std::move(other.public_ip_address)),
      bootstrap(other.bootstrap),
      check_counter(other.check_counter)
{
    // Reset the moved-from object's members to a valid state
    other.sockfd = -1;
    // ... reset other members ...
}

neroshop::Node::~Node() {
    if(sockfd > 0) {
        close(sockfd);
        sockfd = -1;
    }
}

//-----------------------------------------------------------------------------

std::string neroshop::Node::generate_node_id(const std::string& address, int port) {
    // TODO: increase randomness by using a hardware identifier while maintaining a stable node id
    std::string node_info = address + ":" + std::to_string(port);
    std::string hash = neroshop_crypto::sha3_256(node_info);
    return hash.substr(0, NUM_BITS / 4);
}

//-----------------------------------------------------------------------------

// Define the list of bootstrap nodes
std::vector<neroshop::Peer> bootstrap_nodes = {
    {"node.neroshop.org", NEROSHOP_P2P_DEFAULT_PORT},
};

void neroshop::Node::join() {
    if(sockfd < 0) throw std::runtime_error("socket is dead");

    // Bootstrap the DHT node with a set of known nodes
    for (const auto& bootstrap_node : bootstrap_nodes) {
        std::cout << "\033[35;1mJoining bootstrap node - " << bootstrap_node.address << ":" << bootstrap_node.port << "\033[0m\n";

        // Ping each known node to confirm that it is online - the main bootstrapping primitive. If a node replies, and if there is space in the routing table, it will be inserted.
        if(!ping(bootstrap_node.address, bootstrap_node.port)) {
            std::cerr << "ping: failed to ping bootstrap node\n"; continue;
        }
        
        // Add the bootstrap node to routing table (optional) - stores the node in the routing table for later use.
        auto seed_node = std::make_unique<Node>((bootstrap_node.address == "127.0.0.1") ? this->public_ip_address : bootstrap_node.address, bootstrap_node.port, false);
        seed_node->set_bootstrap(true);
        Node& seed_node_ref = *seed_node; // take a reference to the Node object (to avoid segfault)
        //routing_table->add_node(std::move(seed_node)); // seed_node becomes invalid after we move ownership to routing table so it cannot be used
        
        // Send a "find_node" message to the bootstrap node and wait for a response message
        auto nodes = send_find_node(this->id, (bootstrap_node.address == "127.0.0.1") ? "127.0.0.1" : seed_node_ref.get_ip_address(), seed_node_ref.get_port());//std::cout << "Sending find_node message to " << seed_node_ref.get_ip_address() << ":" << seed_node_ref.get_port() << "\n";
        if(nodes.empty()) {
            std::cerr << "find_node: No nodes found\n"; continue;
        }
        
        // Then add nodes to the routing table
        for (auto node : nodes) {
            // Ping the received nodes first
            std::string node_ip = (node->get_ip_address() == this->public_ip_address) ? "127.0.0.1" : node->get_ip_address();
            if(!ping(node_ip, node->get_port())) {
                continue; // Skip the node and continue with the next iteration
            }
            // Process the response and update the routing table if necessary
            routing_table->add_node(std::unique_ptr<neroshop::Node>(node));
        }
    }
    
    // Print the contents of the routing table
    routing_table->print_table();
}

// TODO: create a softer version of the ping-based join() called join_insert() function for storing pre-existing nodes from local database

bool neroshop::Node::ping(const std::string& address, int port) {
    // In a DHT network, new nodes usually ping a known bootstrap node to join the network. The bootstrap node is typically a well-known node in the network that is stable and has a high probability of being online. When a new node pings the bootstrap node, it receives information about other nodes in the network and can start building its routing table.
    // Existing nodes usually ping the nodes closest to them in the keyspace to update their routing tables and ensure they are still live and responsive. In a distributed hash table, the closest nodes are determined using the XOR metric on the node IDs.
    return send_ping(address, port);
}

std::vector<neroshop::Node*> neroshop::Node::find_node(const std::string& target_id, int count) const { 
    if(!routing_table.get()) {
        return {};
    }
    // Get the nodes from the routing table that are closest to the target node
    std::vector<Node*> closest_nodes = routing_table->find_closest_nodes(target_id, count);
    return closest_nodes;
}

std::vector<neroshop::Peer> neroshop::Node::get_peers(const std::string& info_hash) const {
    std::vector<Peer> peers = {};

    // Check if info_hash is in info_hash_peers
    auto info_hash_it = info_hash_peers.find(info_hash);
    if (info_hash_it != info_hash_peers.end()) {
        // If info_hash is in info_hash_peers, get the vector of peers
        peers = info_hash_it->second;
    } else {
        std::vector<Node*> nodes = find_node(info_hash, NEROSHOP_DHT_MAX_CLOSEST_NODES);
        for (Node* node : nodes) {
            // Access the info_hash_peers map for each node and concatenate the vectors of peers
            auto node_it = node->info_hash_peers.find(info_hash);
            if (node_it != node->info_hash_peers.end()) {
                const std::vector<Peer>& node_peers = node_it->second;
                peers.insert(peers.end(), node_peers.begin(), node_peers.end());
            }
        }
    }
    
    return peers;
}

void neroshop::Node::announce_peer(const std::string& info_hash, int port, const std::string& token/*, bool implied_port*/) {

}

void neroshop::Node::add_peer(const std::string& info_hash, const Peer& peer) {
    // Check if the info_hash is already in the info_hash_peers map
    auto it = info_hash_peers.find(info_hash);
    if (it != info_hash_peers.end()) {
        // If the info_hash is already in the map, add the peer to the vector of peers
        it->second.push_back(peer);
    } else {
        // If the info_hash is not in the map, create a new vector of peers and add the peer
        info_hash_peers.emplace(info_hash, std::vector<Peer>{peer});
    }
}

void neroshop::Node::remove_peer(const std::string& info_hash) {
    // Find the info_hash entry in the info_hash_peers map
    auto it = info_hash_peers.find(info_hash);
    if (it != info_hash_peers.end()) {
        // If the info_hash exists, remove the entry from the map
        info_hash_peers.erase(it);
    }
}

int neroshop::Node::put(const std::string& key, const std::string& value) {
    if(!validate(key, value)) {
        return false;
    }
    
    // If data is a duplicate, skip it and return success (true)
    if (has_key(key) && get(key) == value) {
        std::cout << "Data already exists. Skipping ...\n";
        return true;
    }
    
    // If node has the key but the value has been altered, verify data integrity and ownership then update the data
    if (has_key(key) && get(key) != value) {
        std::cout << "Updating value for key (" << key << ")\n";
        return set(key, value);
    }

    data[key] = value;
    return has_key(key); // boolean
}

int neroshop::Node::store(const std::string& key, const std::string& value) {    
    return put(key, value);
}

std::string neroshop::Node::get(const std::string& key) const {    
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "";
}

std::string neroshop::Node::find_value(const std::string& key) const {
    return get(key);
}

int neroshop::Node::remove(const std::string& key) {
    data.erase(key);
    return (data.count(key) == 0); // boolean
}

void neroshop::Node::map(const std::string& key, const std::string& value) {
    if(!validate(key, value)) {
        return;
    }
    
    mapper->add(key, value); // Temporarily stores the mapping in C++ for serialization before permanently adding it to the database
}

int neroshop::Node::set(const std::string& key, const std::string& value) {
    nlohmann::json json = nlohmann::json::parse(value); // Already validated so we just need to parse it without checking for errors
    // Detect when key is the same but the value has changed
    if(has_key(key)) {
        std::string current_value = find_value(key);//std::cout << "preexisting_value: " << preexisting_value << std::endl;
            
        // Compare the preexisting value with the new value
        if (current_value != value) {
            ////std::cout << "Value modification detected. Performing signature verification ...\n";//std::cout << "Value mismatch. Skipping ...\n";//return false;}
            nlohmann::json current_json = nlohmann::json::parse(current_value);
            
            // Verify signature using user's public key (required for account data and listing data)
            if (json.contains("signature")) {
                assert(json["signature"].is_string());
                std::string signature = json["signature"].get<std::string>();//neroshop::base64_decode(json["signature"].get<std::string>());
                
                // READ THIS!!
                // Unfortunately data verification can only be done on the client side since the wallet is used for verification and it only exists on the client side
                // The only way to verify from the daemon backend would be to use the RSA keys instead of Monero keys for signing/verifying
                
                // Get the public key and other account data from the user
                /////std::string public_key = json["public_key"].get<std::string>();//user.get_public_key(); // Get the public key from the user object
                ////std::string user_id = json["monero_address"].get<std::string>();//user.get_id(); // Get the user ID from the user object
        
                ////std::cout << "Verifying existing key's signature ...\n";
                /*bool verified = neroshop_crypto::rsa_public_verify(public_key, user_id, signature);
                if(!verified) {
                    std::cerr << "Verification failed." << std::endl;
                    return false; // Verification failed, return false
                }*/
            }
            
            // Compare dates of new data and old (pre-existing) data - untested
            if(json.contains("last_updated")) {
                assert(json["last_updated"].is_string());
                std::string last_updated = json["last_updated"].get<std::string>();
                
                // Check if current value has a last_updated field too
                if(current_json.contains("last_updated")) {
                    assert(current_json["last_updated"].is_string());
                    std::string current_last_updated = current_json["last_updated"].get<std::string>();
                    // Compare the new json's last_updated timestamp with the current json's own
                    // And choose whichever has the most recent timestamp then exit the function
                    std::string most_recent_timestamp = neroshop_timestamp::get_most_recent_timestamp(last_updated, current_last_updated);
                    // If this node has the up-to-date value, return true as there is no need to update
                    if(most_recent_timestamp == current_last_updated) {
                        std::cout << "Value for key (" << key << ") is already up-to-date" << std::endl;
                        return true;
                    }
                }
                // If current value does not have a last_updated field 
                // then it means it's probably outdated, so do nothing.
                // It will be replaced with the new value at the end of the scope
            }
        }
    }
    
    data[key] = value;
    return has_key(key); // boolean
}

//-------------------------------------------------------------------------------------

void neroshop::Node::persist_routing_table(const std::string& address, int port) {
    if(!is_bootstrap_node()) return; // Regular nodes cannot run this function
    
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    database->execute("BEGIN;");
    
    if(!database->table_exists("routing_table")) { // or simply "nodes"
        database->execute("CREATE TABLE routing_table("
        "ip_address TEXT, port INTEGER, UNIQUE(ip_address, port));");
    }
    
    database->execute_params("INSERT INTO routing_table (ip_address, port) VALUES (?1, ?2);", { address, std::to_string(port) });
    
    database->execute("COMMIT;");
}

void neroshop::Node::rebuild_routing_table() {
    if(!is_bootstrap_node()) return; // Regular nodes cannot run this function
    
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    if(!database->table_exists("routing_table")) {
        return; // Table does not exist, exit function
    }
    // Prepare statement
    std::string command = "SELECT ip_address, port FROM routing_table;";
    sqlite3_stmt * stmt = nullptr;
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cout << "\033[91msqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())) << "\033[0m" << std::endl;
        return;
    }
    // Check if there is any data returned by the statement
    if(sqlite3_column_count(stmt) > 0) {
        std::cout << "\033[35;1mRebuilding routing table from backup ...\033[0m" << std::endl;
    }
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        std::string ip_address; int port = 0;
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            if(column_value == "NULL") continue;
            if(i == 0) ip_address = column_value;
            if(i == 1) port = sqlite3_column_int(stmt, i);//std::stoi(column_value);
            if(port == 0) continue;
            
            if(!ping((ip_address == this->public_ip_address) ? "127.0.0.1" : ip_address, port)) {
                std::cerr << "ping: failed to ping bootstrap node\n"; 
                // Remove unresponsive nodes from database
                database->execute_params("DELETE FROM routing_table WHERE ip_address = ?1 AND port = ?2", { ip_address, std::to_string(port) });
                continue;
            }
            
            auto node = std::make_unique<Node>(ip_address, port, false);
            routing_table->add_node(std::move(node));
        }
    }
    // Finalize statement
    sqlite3_finalize(stmt);
}

//-------------------------------------------------------------------------------------

std::vector<uint8_t> neroshop::Node::send_query(const std::string& address, uint16_t port, const std::vector<uint8_t>& message, int recv_timeout) {
    // Step 2: Resolve the hostname and construct a destination address
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use IPv4
    hints.ai_socktype = SOCK_DGRAM; // use UDP //SOCK_STREAM; // use TCP
    if (getaddrinfo(address.c_str(), std::to_string(port ? port : NEROSHOP_P2P_DEFAULT_PORT).c_str(), &hints, &res) != 0) {
        std::cerr << "Error resolving hostname" << std::endl; // probably the wrong family
        return {};
    }

    if (res == NULL) {
        std::cerr << "Error resolving hostname" << std::endl;
        return {};
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = hints.ai_family;
    memcpy(&dest_addr, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    //--------------------------------------------
    // Step 3: Create a new socket descriptor and send the ping message to the server
    // Note: This is a separate socket used for actively sending/receiving queries while the main socket, which is non-blocking is used for listening/responding to messages
    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        return {};
    }

    if (sendto(socket_fd/*sockfd*/, message.data(), message.size(), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("sendto");
        close(socket_fd);
        return {};
    }
    //--------------------------------------------
    // Set a timeout for the receive operation
    // Note: Setting a timeout is better than setting the socket to non-blocking because if the socket is non-blocking then it will never receive the pong because it does not wait for the pong message so it fails immediately (returns immediately, regardless of whether data is available or not).
    struct timeval timeout;
    timeout.tv_sec = recv_timeout; // Timeout in seconds
    timeout.tv_usec = 0;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        return {};
    }
    //--------------------------------------------
    // Step 4: Receive the pong message from the server
    std::vector<uint8_t> receive_buffer(NEROSHOP_RECV_BUFFER_SIZE);
    socklen_t fromlen = sizeof(struct sockaddr_in);
    int bytes_received = recvfrom(socket_fd/*sockfd*/, receive_buffer.data(), receive_buffer.size(), 0,
                                  (struct sockaddr*)&dest_addr, &fromlen);
    if (bytes_received < 0) {
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("recvfrom");
            close(socket_fd);
            return {};
        }
        
        // No data available at the moment, you can handle it based on your needs
        // For example, you can retry receiving or return an empty vector
        
        close(socket_fd);
        return {};
    }
    receive_buffer.resize(bytes_received);
    close(socket_fd);

    return receive_buffer;
}

//-----------------------------------------------------------------------------

bool neroshop::Node::send_ping(const std::string& address, int port) {
    // Create the ping message
    std::string transaction_id = msgpack::generate_transaction_id();
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "ping";
    query_object["args"]["id"] = this->id;
    query_object["args"]["ephemeral_port"] = get_port(); // for testing on local network. This cannot be removed since the two primary sockets used in the protocol have different ports with the "ephemeral_port" being the actual port
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    
    auto ping_message = nlohmann::json::to_msgpack(query_object);
    //--------------------------------------------
    auto receive_buffer = send_query(address, port, ping_message, NEROSHOP_DHT_PING_MESSAGE_TIMEOUT);
    //--------------------------------------------
    // Parse the pong message and extract the transaction ID and response fields
    nlohmann::json pong_message;
    try {
        pong_message = nlohmann::json::from_msgpack(receive_buffer);
    } catch (const std::exception& e) {
        std::cerr << "Node \033[91m" << address << ":" << port << "\033[0m did not respond" << std::endl;
        return false;
    }
    if (!pong_message.contains("response") || pong_message.contains("error")) {
        std::cerr << "Received invalid pong message" << std::endl;
        return false;
    }
    std::cout << "\033[32m" << pong_message.dump() << "\033[0m\n";
    std::string received_transaction_id = pong_message["tid"].get<std::string>();
    auto response_object = pong_message["response"];
    std::string response_id = response_object["id"].get<std::string>();
    
    // Check that the pong message corresponds to the ping message
    if (received_transaction_id != transaction_id) {
        std::cerr << "Received pong message with incorrect transaction ID" << std::endl;
        return false;
    }

    return true;
}

std::vector<neroshop::Node*> neroshop::Node::send_find_node(const std::string& target_id, const std::string& address, uint16_t port) {
    std::string transaction_id = msgpack::generate_transaction_id();

    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "find_node";
    query_object["args"]["id"] = this->id;
    query_object["args"]["target"] = target_id;
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    
    auto find_node_message = nlohmann::json::to_msgpack(query_object);
    //---------------------------------------------------------
    auto receive_buffer = send_query(address, port, find_node_message);
    //---------------------------------------------------------
    // Parse nodes message
    nlohmann::json nodes_message;
    try {
        nodes_message = nlohmann::json::from_msgpack(receive_buffer);
    } catch (const std::exception& e) {
        std::cerr << "Node \033[91m" << address << ":" << port << "\033[0m did not respond" << std::endl;
        return {};
    }
    std::cout << "\033[32m" << nodes_message.dump() << "\033[0m\n";
    // Create node vector and store nodes from the message inside the vector
    std::vector<Node*> nodes;
    if (nodes_message.contains("response") && nodes_message["response"].contains("nodes")) {
        for (auto& node_json : nodes_message["response"]["nodes"]) {
            if (node_json.contains("ip_address") && node_json.contains("port")) {
                std::string ip_address = node_json["ip_address"];
                uint16_t port = node_json["port"];
                Node* node = new Node(ip_address, port, false);
                if (node->id != this->id && !routing_table->has_node(node->id)) { // add node to vector only if it's not the current node
                    nodes.push_back(node);
                }
            }
        }
    }
    return nodes;
}

void neroshop::Node::send_get_peers(const std::string& info_hash) {

    std::string transaction_id = msgpack::generate_transaction_id();
    
    /*bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "get_peers";
    
    std::string get_peers_message = bencode::encode(query);*/
    //-----------------------------------------------------
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "get_peers";
    query_object["args"]["id"] = this->id;
    query_object["args"]["info_hash"] = info_hash;
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    //-----------------------------------------------
    // socket sendto and recvfrom here
    //auto receive_buffer = send_query(address, port, _message);
    //-----------------------------------------------
    // process the response here
    // ...    
    //-----------------------------------------------
    // get result of put request
    // ...    
}

void neroshop::Node::send_announce_peer(const std::string& info_hash, int port, const std::string& token) {
    // announce_peer is called by a client to announce that it is downloading a file with a specific infohash and is now a potential peer for that file. This is typically called when a client starts downloading a file or completes downloading a file and wants to start seeding it.
    // When a client calls announce_peer, it sends an announce_peer query to the DHT network, which contains the client's node ID, the infohash of the file, and the client's IP address and port number. If the query is successful, the DHT network will store the client's IP address and port number in the list of peers for the specified infohash.
    // Other clients who are also downloading or seeding the same file can then query the DHT network for a list of peers for that infohash. The DHT network will respond with a list of IP addresses and port numbers of all the peers who have announced themselves for that infohash. The requesting client can then use this list to connect to other peers and start downloading or uploading data.
    std::string transaction_id = msgpack::generate_transaction_id();
    
    /*bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "announce_peer";
    
    std::string announce_peer_message = bencode::encode(query);*/
    //---------------------------------------------------------
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "announce_peer";
    query_object["args"]["id"] = this->id;
    query_object["args"]["info_hash"] = info_hash;
    query_object["args"]["port"] = port; // the port of the peer that is announcing itself
    query_object["args"]["token"] = token;
    query_object["args"]["implied_port"] = (port != 0);//implied_port; // optional // set to 1 if the port number is included in the peer list, and 0 otherwise. // refers to the port of the peer that is announcing itself, not the port of the node that receives the announcement // the value of implied_port should be set based on whether the port number is included in the peer list or not, and it should not be set to this->port.
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    //-----------------------------------------------    
    // send the query to the nodes in the routing table
    //auto receive_buffer = send_query(address, port, _message);  
    //-----------------------------------------------    
    // process the response
    // ...
    //-----------------------------------------------
    // get result of put request
    // ...        
}

void neroshop::Node::send_add_peer(const std::string& info_hash, const Peer& peer) {

}

int neroshop::Node::send_put(const std::string& key, const std::string& value) {
    
    nlohmann::json query_object;
    query_object["query"] = "put";
    query_object["args"]["id"] = this->id;
    query_object["args"]["key"] = key;
    query_object["args"]["value"] = value;
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    //-----------------------------------------------
    // Determine which nodes get to put the key-value data in their hash table
    std::vector<Node *> closest_nodes = find_node(key, NEROSHOP_DHT_REPLICATION_FACTOR); // 5=replication factor
    
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(closest_nodes.begin(), closest_nodes.end(), rng);
    //-----------------------------------------------
    // Keep track of the number of nodes to which put messages have been sent
    size_t nodes_sent_count = 0;
    std::unordered_set<Node*> sent_nodes;
    std::unordered_set<Node*> failed_nodes;
    // Send put message to the closest nodes
    for(auto const& node : closest_nodes) {
        std::string transaction_id = msgpack::generate_transaction_id();
        query_object["tid"] = transaction_id; // tid should be unique for each put message
        std::vector<uint8_t> put_message = nlohmann::json::to_msgpack(query_object);
    
        std::string node_ip = (node->get_ip_address() == this->public_ip_address) ? "127.0.0.1" : node->get_ip_address();
        int node_port = node->get_port();
        std::cout << "Sending put request to \033[36m" << node_ip << ":" << node_port << "\033[0m\n";
        auto receive_buffer = send_query(node_ip, node_port, put_message);
        // Process the response here
        nlohmann::json put_response_message;
        try {
            put_response_message = nlohmann::json::from_msgpack(receive_buffer);
        } catch (const std::exception& e) {
            std::cerr << "Node \033[91m" << node_ip << ":" << node_port << "\033[0m did not respond" << std::endl;
            node->check_counter += 1;
            failed_nodes.insert(node);
            continue; // Continue with the next closest node if this one fails
        }   
        // Add the node to the sent_nodes set
        sent_nodes.insert(node);
        // Show response and increase count
        std::cout << ((put_response_message.contains("error")) ? ("\033[91m") : ("\033[32m")) << put_response_message.dump() << "\033[0m\n";
        nodes_sent_count++;
    }
    //-----------------------------------------------
    // Handle the case when there are fewer closest nodes than NEROSHOP_DHT_REPLICATION_FACTOR - this most likely means that there are not enough nodes in the network
    if (closest_nodes.size() < NEROSHOP_DHT_REPLICATION_FACTOR) return nodes_sent_count;
    //-----------------------------------------------
    // If the desired number of nodes is not reached due to non-responses, replace failed nodes with new nodes and continue sending put messages
    if (nodes_sent_count < NEROSHOP_DHT_REPLICATION_FACTOR) {
        size_t remaining_nodes = NEROSHOP_DHT_REPLICATION_FACTOR - nodes_sent_count;
        std::cout << "Nodes remaining: " << remaining_nodes << " out of " << NEROSHOP_DHT_REPLICATION_FACTOR << "\n";
        std::cout << "Routing table total node count: " << routing_table->get_node_count() << "\n";
        
        std::vector<Node*> all_nodes = find_node(key, routing_table->get_node_count());
        std::vector<Node*> replacement_nodes;
        
        // Iterate over all the nodes in the routing table
        for (const auto& node : all_nodes) {
            if (std::find(closest_nodes.begin(), closest_nodes.end(), node) == closest_nodes.end() &&
                std::find(failed_nodes.begin(), failed_nodes.end(), node) == failed_nodes.end() &&
                sent_nodes.find(node) == sent_nodes.end()) {
                    replacement_nodes.push_back(node);
            }
        }
        
        if (replacement_nodes.size() < remaining_nodes) {
            // Handle the case where there are not enough replacement nodes available
            std::cerr << "Not enough replacement nodes available.\n";
        } else {
            // Select the required number of replacement nodes
            replacement_nodes.resize(remaining_nodes);

            // Send put messages to the replacement nodes
            for (const auto& replacement_node : replacement_nodes) {
                std::string transaction_id = msgpack::generate_transaction_id();
                query_object["tid"] = transaction_id;
                std::vector<uint8_t> put_message = nlohmann::json::to_msgpack(query_object);

                std::string node_ip = (replacement_node->get_ip_address() == this->public_ip_address) ? "127.0.0.1" : replacement_node->get_ip_address();
                int node_port = replacement_node->get_port();
                std::cout << "Sending put request to \033[36m" << node_ip << ":" << node_port << "\033[0m\n";
                auto receive_buffer = send_query(node_ip, node_port, put_message);
                // Process the response and update the nodes_sent_count and sent_nodes accordingly
                nlohmann::json put_response_message;
                try {
                    put_response_message = nlohmann::json::from_msgpack(receive_buffer);
                } catch (const std::exception& e) {
                    std::cerr << "Node \033[91m" << node_ip << ":" << node_port << "\033[0m did not respond" << std::endl;
                    replacement_node->check_counter += 1;
                    continue; // Continue with the next replacement node if this one fails
                }   
                // Show response and increase count
                std::cout << ((put_response_message.contains("error")) ? ("\033[91m") : ("\033[32m")) << put_response_message.dump() << "\033[0m\n";
                nodes_sent_count++;
            }
        }
    }
    //-----------------------------------------------
    return nodes_sent_count;
}

int neroshop::Node::send_store(const std::string& key, const std::string& value) {
    return send_put(key, value);
}

std::string neroshop::Node::send_get(const std::string& key) {
    std::string value;

    nlohmann::json query_object;
    query_object["query"] = "get";
    query_object["args"]["id"] = this->id;
    query_object["args"]["key"] = key;
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    
    std::vector<uint8_t> get_message = nlohmann::json::to_msgpack(query_object);
    //-----------------------------------------------
    std::vector<Node *> closest_nodes = find_node(key, NEROSHOP_DHT_MAX_CLOSEST_NODES);
    
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(closest_nodes.begin(), closest_nodes.end(), rng);
    //-----------------------------------------------
    // First, check to see if we have the key before performing any other operations
    if(has_key(key)) return find_value(key);
    //-----------------------------------------------
    // Send get message to the closest nodes
    for(auto const& node : closest_nodes) {
        std::string transaction_id = msgpack::generate_transaction_id();
        query_object["tid"] = transaction_id; // tid should be unique for each get message
        std::vector<uint8_t> get_message = nlohmann::json::to_msgpack(query_object);
    
        std::string node_ip = (node->get_ip_address() == this->public_ip_address) ? "127.0.0.1" : node->get_ip_address();
        int node_port = node->get_port();
        std::cout << "Sending get request to \033[36m" << node_ip << ":" << node_port << "\033[0m\n";
        auto receive_buffer = send_query(node_ip, node_port, get_message, 2);
        // Process the response here
        nlohmann::json get_response_message;
        try {
            get_response_message = nlohmann::json::from_msgpack(receive_buffer);
        } catch (const std::exception& e) {
            std::cerr << "Node \033[91m" << node_ip << ":" << node_port << "\033[0m did not respond" << std::endl;
            node->check_counter += 1;
            continue; // Continue with the next closest node if this one fails
        }   
        // Show response and handle the retrieved value
        std::cout << ((get_response_message.contains("error")) ? ("\033[91m") : ("\033[32m")) << get_response_message.dump() << "\033[0m\n";
        if(get_response_message.contains("error")) {
            continue; // Skip if error
        }
        if (get_response_message.contains("response") && get_response_message["response"].contains("value")) {
            std::string retrieved_value = get_response_message["response"]["value"].get<std::string>();
            if (!retrieved_value.empty()) {
                value = retrieved_value;
                break; // Exit the loop if a value is found
            }
        }
    }
    //-----------------------------------------------
    return value;
}

std::string neroshop::Node::send_find_value(const std::string& key) {
    return send_get(key);
}

void neroshop::Node::send_remove(const std::string& key) {
    std::string transaction_id = msgpack::generate_transaction_id();
    
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "remove";
    query_object["args"]["key"] = key;
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    
    // ...
}

void neroshop::Node::send_map(const std::string& address, int port) {
    nlohmann::json query_object;
    query_object["query"] = "map";
    query_object["args"]["id"] = this->id;
    query_object["version"] = std::string(NEROSHOP_DHT_VERSION);
    
    bool map_sent = false;
    for (const auto& pair : data) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;
        
        query_object["args"]["key"] = key;
        query_object["args"]["value"] = value;
        std::string transaction_id = msgpack::generate_transaction_id();
        query_object["tid"] = transaction_id; // tid should be unique for each map message
        std::vector<uint8_t> map_message = nlohmann::json::to_msgpack(query_object);

        auto receive_buffer = send_query(address, port, map_message);
        // Process the response here
        nlohmann::json map_response_message;
        try {
            map_response_message = nlohmann::json::from_msgpack(receive_buffer);
            map_sent = true;
        } catch (const std::exception& e) {
            std::cerr << "Node \033[91m" << address << ":" << port << "\033[0m did not respond to send_map" << std::endl;
        }
        // Show response
        std::cout << ((map_response_message.contains("error")) ? ("\033[91m") : ("\033[92m")) << map_response_message.dump() << "\033[0m\n";
    }
    if(map_sent && !data.empty()) std::cout << "\033[93mIndexing data distributed to " << address << ":" << port << "\033[0m\n";
}

//-----------------------------------------------------------------------------

void neroshop::Node::republish() {
    for (const auto& pair : data) {
        const std::string& key = pair.first;
        const std::string& value = pair.second;

        send_put(key, value);
    }
    if(!data.empty()) std::cout << "\033[93mData republished\033[0m\n";
}

//-----------------------------------------------------------------------------

bool neroshop::Node::validate(const std::string& key, const std::string& value) {
    assert(key.length() == 64 && "Key length is not 64 characters");
    assert(!value.empty() && "Value is empty");
    
    // Ensure that the value is valid JSON
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(value);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false; // Invalid value, return false
    }
    
    // Make sure value contains a metadata field
    if(!json.contains("metadata")) {
        std::cerr << "No metadata found\n";
        return false;
    }
    
    // Check whether data is expired so we don't store it
    if(json.contains("expiration_date")) {
        assert(json["expiration_date"].is_string());
        std::string expiration_date = json["expiration_date"].get<std::string>();
        if(neroshop_timestamp::is_expired(expiration_date)) {
            std::cerr << "Data has expired (exp date: " << expiration_date << " UTC)\n";
            // Notify the other nodes that this data has expired (so that once they receive a put with the expired data, it will be removed from their local hash table as soon as it goes through the validate function)
            ////send_put(key, value); // this should propagate the expiration information to other nodes in the DHT until the expired data is removed once and for all. Hmmm this could cause an endless loop ...
            // Remove the data from hash table if it was previously stored
            if(has_key(key)) {
                if(remove(key) == true) {
                    std::cout << "Expired data with key (" << key << ") has been removed from hash table\n";
                }
            }
            return false;
        }
    }
    
    return true;
}

//-----------------------------------------------------------------------------

void neroshop::Node::periodic_refresh() {
    while (true) {
        {
            // Acquire the lock before accessing the data
            std::shared_lock<std::shared_mutex> read_lock(node_read_mutex);
            
            // Perform periodic republishing here
            // This code will run concurrently with the listen/receive loop
            if(!data.empty()) {
                std::cout << "\033[34;1mPerforming periodic refresh\033[0m\n";
            }
            
            republish();
            
            // read_lock is released here
        }
        
        // Sleep for a specified interval
        std::this_thread::sleep_for(std::chrono::hours(NEROSHOP_DHT_REPUBLISH_INTERVAL));
    }
}

//-----------------------------------------------------------------------------

void neroshop::Node::periodic_check() {
    ////std::vector<std::string> dead_node_ids {};
    while(true) {
        {
        // Acquire the lock before accessing the routing table
        std::shared_lock<std::shared_mutex> read_lock(node_read_mutex);
        // Perform periodic checks here
        // This code will run concurrently with the listen/receive loop
        for (auto& bucket : routing_table->buckets) {
            for (auto& node : bucket.second) {
                if (node.get() == nullptr) continue; // It's possible that the invalid node object is being accessed or modified by another thread concurrently, even though its already been removed from the routing_table
                std::string node_ip = (node->get_ip_address() == this->public_ip_address) ? "127.0.0.1" : node->get_ip_address();
                uint16_t node_port = node->get_port();
                
                // Skip the bootstrap nodes from the periodic checks
                if (node->is_bootstrap_node()) continue;
                
                std::cout << "Performing periodic check on \033[34m" << node_ip << ":" << node_port << "\033[0m\n";
                
                // Perform the liveness check on the current node
                bool pinged = ping(node_ip, node_port);
                
                // Update the liveness status of the node in the routing table
                node->check_counter = pinged ? 0 : (node->check_counter + 1);
                std::cout << "Health check failures: " << node->check_counter << (" (" + node->get_status_as_string() + ")") << "\n";
                
                // If node is dead, remove it from the routing table
                if(node->is_dead()) {
                    std::cout << "\033[0;91m" << node->public_ip_address << ":" << node_port << "\033[0m marked as dead\n";
                    if(routing_table->has_node(node->public_ip_address, node_port)) {
                        ////dead_node_ids.push_back(node->get_id());
                        routing_table->remove_node(node->public_ip_address, node_port); // Already has internal write_lock
                    }
                }
            }
        }
            // read_lock is released here
        }
        
        /*on_dead_node(dead_node_ids);
        // Clear the vector for the next iteration
        dead_node_ids.clear();*/
        // Sleep for a specified interval
        std::this_thread::sleep_for(std::chrono::seconds(NEROSHOP_DHT_PERIODIC_CHECK_INTERVAL));
    }
}

//-----------------------------------------------------------------------------

/*bool neroshop::Node::on_keyword_blocked(const nlohmann::json& value) {
    // Note: This code is in the testing stage
    // Block certain keywords/search terms from listings
    assert(json.contains("metadata"));
    if(json["metadata"] == "listing") {
        //--------------------------------------------
        // Block categories marked as "Illegal"
        if(json["product"]["category"] == "Illegal" ||
            json["product"]["subcategory"] == "Illegal") {
            std::cout << "Illegal product blocked\n";
            return true; // The category is blocked, do not insert the data
        }
        //--------------------------------------------
        // Block certain tags
        std::vector<std::string> blocked_tags = { "heroin", "meth", "cp", "child porn" };
        
        if(json["product"].contains("tags")) {
            assert(json["product"]["tags"].is_array());
            std::vector<std::string> product_tags = json["product"]["tags"].get<std::vector<std::string>>();
            // Check if any of the product tags match the blocked tags
            bool has_blocked_tag = std::any_of(product_tags.begin(), product_tags.end(), [&](const std::string& tag) {
                return std::find(blocked_tags.begin(), blocked_tags.end(), tag) != blocked_tags.end();
            });
            // Print the result
            if (has_blocked_tag) {
                std::cout << "Product contains a blocked tag." << std::endl;
                return true;
            } else {
                std::cout << "Product does not contain any blocked tags." << std::endl;
            }
        }
        //--------------------------------------------
        // Block other search terms
        //--------------------------------------------
    }
    return false;
}*/

//-----------------------------------------------------------------------------

void neroshop::Node::on_ping(const std::vector<uint8_t>& buffer, const struct sockaddr_in& client_addr) {
    if (buffer.size() > 0) {
        nlohmann::json message = nlohmann::json::from_msgpack(buffer);
        if (message.contains("query") && message["query"] == "ping") {
            std::string sender_id = message["args"]["id"].get<std::string>();
            std::string sender_ip = inet_ntoa(client_addr.sin_addr);
            uint16_t sender_port = (message["args"].contains("ephemeral_port")) ? (uint16_t)message["args"]["ephemeral_port"] : ntohs(client_addr.sin_port);//NEROSHOP_P2P_DEFAULT_PORT;
            bool node_exists = routing_table->has_node((sender_ip == "127.0.0.1") ? this->public_ip_address : sender_ip, sender_port);
            if (!node_exists) {
                auto node_that_pinged = std::make_unique<Node>((sender_ip == "127.0.0.1") ? this->public_ip_address : sender_ip, sender_port, false);
                if(!node_that_pinged->is_bootstrap_node()) { // To prevent the bootstrap node from being stored in the routing table
                    routing_table->add_node(std::move(node_that_pinged)); // Already has internal write_lock
                    persist_routing_table((sender_ip == "127.0.0.1") ? this->public_ip_address : sender_ip, sender_port);
                    routing_table->print_table();
                    // Redistribute your indexing data to the new node that recently joined the network to make product/service listings more easily discoverable by the new node
                    send_map((sender_ip == this->public_ip_address) ? "127.0.0.1" : sender_ip, sender_port);
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------

/*void neroshop::Node::on_dead_node(const std::vector<std::string>& node_ids) {
    for (const auto& dead_node_id : node_ids) {
        //std::cout << "Processing dead node ID: " << dead_node_id << std::endl;

        auto closest_nodes = find_node(dead_node_id, NEROSHOP_DHT_REPLICATION_FACTOR);
        
        for (auto& routing_table_node : closest_nodes) {
            if (routing_table_node == nullptr) continue;
            std::string routing_table_node_ip = (routing_table_node->get_ip_address() == this->public_ip_address) ? "127.0.0.1" : routing_table_node->get_ip_address();
            uint16_t routing_table_node_port = routing_table_node->get_port();
            
            // Skip the bootstrap nodes (as they are not involved in the replacement of dead nodes)
            if (routing_table_node->is_bootstrap_node()) continue;
                
            // Send find_node to make up for dead nodes
            auto nodes = send_find_node(dead_node_id, routing_table_node_ip, routing_table_node_port);
            if(nodes.empty()) {
                std::cerr << "find_node: No nodes found\n"; continue;
            }
                
            // Then add nodes to the routing table
            for (auto node : nodes) {
                if (node->get_id() == dead_node_id) continue; // ignore the dead node
                // Ping the received nodes before adding them
                std::string node_ip = (node->get_ip_address() == this->public_ip_address) ? "127.0.0.1" : node->get_ip_address();
                uint16_t node_port = node->get_port();
                if(!ping(node_ip, node_port)) {
                    node->check_counter++;
                    continue; // Skip the node and continue with the next iteration
                }
                // Update the routing table with the received nodes
                if(!routing_table->has_node(node->public_ip_address, node_port)) {
                    routing_table->add_node(std::unique_ptr<neroshop::Node>(node));
                }
            }    
        }
    }
}*/

//-----------------------------------------------------------------------------

void neroshop::Node::run() {
    
    run_optimized();
    return;
    
    // Start a separate thread for periodic checks and republishing
    std::thread periodic_check_thread([this]() { periodic_check(); });
    std::thread periodic_refresh_thread([this]() { periodic_refresh(); });
    
    while (true) {
        std::vector<uint8_t> buffer(NEROSHOP_RECV_BUFFER_SIZE);
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                      (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytes_received == -1 && errno == EAGAIN) {
            // No data available, continue loop
            continue;
        }
        else if (bytes_received < 0) {
            perror("recvfrom");
        }

        // Resize the buffer to the actual number of received bytes
        buffer.resize(bytes_received);

        if (buffer.size() > 0) std::cout << "Received request from \033[0;36m" << inet_ntoa(client_addr.sin_addr) << "\033[0m\n";
        
        // Create a lambda function to handle the request
        auto handle_request_fn = [=]() {
            // Acquire the lock before accessing the routing table
            std::shared_lock<std::shared_mutex> read_lock(node_read_mutex);
            // Process the message
            std::vector<uint8_t> response = neroshop::msgpack::process(buffer, *this, false);

            // Send the response
            int bytes_sent = sendto(sockfd, response.data(), response.size(), 0,
                                (struct sockaddr*)&client_addr, client_addr_len);
            if (bytes_sent < 0) {
                perror("sendto");
            }
        
            // Add the node that pinged this node to the routing table
            on_ping(buffer, client_addr);
        };
        
        // Create a detached thread to handle the request
        std::thread request_thread(handle_request_fn);
        request_thread.detach();
    }
    // Wait for the periodic threads to finish
    periodic_check_thread.join();
    periodic_refresh_thread.join();
}

// This uses less CPU
void neroshop::Node::run_optimized() {
    // Start a separate thread for periodic checks and republishing
    std::thread periodic_check_thread([this]() { periodic_check(); });
    std::thread periodic_refresh_thread([this]() { periodic_refresh(); });

    while (true) {
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sockfd, &read_set);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;  // Timeout of 100ms

        int ready = select(sockfd + 1, &read_set, nullptr, nullptr, &timeout);
        if (ready == -1) {
            perror("select");
            // Handle the error
            // ...
            break;
        } else if (ready == 0) {
            // No data available within the timeout period, continue the loop
            continue;
        }

        if (FD_ISSET(sockfd, &read_set)) {
            std::vector<uint8_t> buffer(NEROSHOP_RECV_BUFFER_SIZE);
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), MSG_DONTWAIT,
                                          (struct sockaddr*)&client_addr, &client_addr_len);
            if (bytes_received == -1 && errno == EAGAIN) {
                // No data available, continue loop
                continue;
            }
            else if (bytes_received < 0) {
                perror("recvfrom");
            }
            
            if (bytes_received > 0) {
                // Resize the buffer to the actual number of received bytes
                buffer.resize(bytes_received);

                if (buffer.size() > 0) std::cout << "Received request from \033[0;36m" << inet_ntoa(client_addr.sin_addr) << "\033[0m\n";
                
                // Create a lambda function to handle the request
                auto handle_request_fn = [=]() {
                    // Acquire the lock before accessing the routing table
                    std::shared_lock<std::shared_mutex> read_lock(node_read_mutex);
                    // Process the message
                    std::vector<uint8_t> response = neroshop::msgpack::process(buffer, *this, false);

                    // Send the response
                    int bytes_sent = sendto(sockfd, response.data(), response.size(), 0,
                                    (struct sockaddr*)&client_addr, client_addr_len);
                    if (bytes_sent < 0) {
                        perror("sendto");
                    }
                
                    // Add the node that pinged this node to the routing table
                    on_ping(buffer, client_addr);
                };
                
                // Create a detached thread to handle the request
                std::thread request_thread(handle_request_fn);
                request_thread.detach();
            }
        }
    }         
    // Wait for the periodic threads to finish
    periodic_check_thread.join();    
    periodic_refresh_thread.join();
}

//-----------------------------------------------------------------------------

std::string neroshop::Node::get_id() const {
    return id;
}

std::string neroshop::Node::get_ip_address() const {
    const int ADDRSTRLEN = (storage.ss_family == AF_INET6) ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN;
    char ip_address[ADDRSTRLEN] = {0};

    if(storage.ss_family == AF_INET) {
        strcpy(ip_address, inet_ntoa(sockin.sin_addr));//std::string ip_address = inet_ntoa(sockin.sin_addr);
    } 
    if(storage.ss_family == AF_INET6) {
        inet_ntop(storage.ss_family, &(sockin6.sin6_addr), ip_address, ADDRSTRLEN);
    }

    return std::string(ip_address);
}

std::string neroshop::Node::get_local_ip_address() const {
    return get_ip_address();
}

std::string neroshop::Node::get_device_ip_address() const {
    std::future<std::string> result = std::async(std::launch::async, neroshop::get_device_ip_address);
    return result.get();
}

std::string neroshop::Node::get_public_ip_address() const {
    std::future<std::string> result = std::async(std::launch::async, neroshop::get_public_ip_address);
    return result.get();
}

uint16_t neroshop::Node::get_port() const {
    uint16_t port = 0;
    
    if(storage.ss_family == AF_INET) {
        port = ntohs(sockin.sin_port);
    } 
    if(storage.ss_family == AF_INET6) {
        port = ntohs(sockin6.sin6_port);
    }
    if(port == 0) {
        // Get the actual port number used by the socket
        struct sockaddr_in local_addr;
        socklen_t local_addr_len = sizeof(local_addr);
        getsockname(sockfd, (struct sockaddr*)&local_addr, &local_addr_len);
        port = ntohs(local_addr.sin_port);
    }    
    return port;
}

/*neroshop::Server * neroshop::Node::get_server() const {
    return server.get();
}*/

neroshop::RoutingTable * neroshop::Node::get_routing_table() const {
    return routing_table.get();
}

int neroshop::Node::get_peer_count() const {
    return routing_table->get_node_count();
}

int neroshop::Node::get_active_peer_count() const {
    int active_count = 0;
    for (auto& bucket : routing_table->buckets) {
        for (auto& node : bucket.second) {
            if (node.get() == nullptr) continue;
            if (node->get_status() == NodeStatus::Active) {
                active_count++;
            }
        }
    }
    return active_count;
}

int neroshop::Node::get_idle_peer_count() const {
    int idle_count = 0;
    for (auto& bucket : routing_table->buckets) {
        for (auto& node : bucket.second) {
            if (node.get() == nullptr) continue;
            if (node->get_status() == NodeStatus::Idle) {
                idle_count++;
            }
        }
    }
    return idle_count;
}

neroshop::NodeStatus neroshop::Node::get_status() const {
    if(check_counter == 0) return NodeStatus::Active;
    if(check_counter <= (NEROSHOP_DHT_MAX_HEALTH_CHECKS - 1)) return NodeStatus::Idle;
    if(check_counter >= NEROSHOP_DHT_MAX_HEALTH_CHECKS) return NodeStatus::Inactive;
    return NodeStatus::Inactive;
}

std::string neroshop::Node::get_status_as_string() const {
    if(check_counter == 0) return "Active";
    if(check_counter <= (NEROSHOP_DHT_MAX_HEALTH_CHECKS - 1)) return "Idle";
    if(check_counter >= NEROSHOP_DHT_MAX_HEALTH_CHECKS) return "Inactive";
    return "Unknown";
}

std::vector<std::string> neroshop::Node::get_keys() const {
    std::vector<std::string> keys;

    for (const auto& pair : data) {
        keys.push_back(pair.first);
    }

    return keys;
}

std::vector<std::pair<std::string, std::string>> neroshop::Node::get_data() const {
    std::vector<std::pair<std::string, std::string>> data_vector;

    for (const auto& pair : data) {
        data_vector.push_back(pair);
    }

    return data_vector;
}

/*const std::unordered_map<std::string, std::string>& neroshop::Node::get_data() const {
    return data;
}*/

//-----------------------------------------------------------------------------

bool neroshop::Node::has_key(const std::string& key) const {
    return (data.count(key) > 0);
}

bool neroshop::Node::has_value(const std::string& value) const {
    for (const auto& pair : data) {
        if (pair.second == value) {
            return true;
        }
    }
    return false;
}

bool neroshop::Node::is_hardcoded(const std::string& address, uint16_t port) {
    for (const auto& bootstrap_node : bootstrap_nodes) {
        if (neroshop::ip::resolve(bootstrap_node.address) == address && bootstrap_node.port == port) {
            return true;
        }
    }
    return false;
}

bool neroshop::Node::is_bootstrap_node() const {
    return (bootstrap == true) || is_hardcoded(this->public_ip_address, this->get_port());
}

bool neroshop::Node::is_dead() const {
    return (check_counter >= NEROSHOP_DHT_MAX_HEALTH_CHECKS);
}

//-----------------------------------------------------------------------------

void neroshop::Node::set_bootstrap(bool bootstrap) {
    this->bootstrap = bootstrap;
}

