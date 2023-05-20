#include "node.hpp"

#include "kademlia.hpp"
#include "../../crypto/sha3.hpp"
#include "routing_table.hpp"
#include "../transport/ip_address.hpp"
#include "../messages/msgpack.hpp"
#include "../../version.hpp"

#include <nlohmann/json.hpp>

#include <cstring> // memset
#include <future>
#include <iomanip> // std::set*
#include <cassert>
#include <thread>

namespace neroshop_crypto = neroshop::crypto;

neroshop::Node::Node(const std::string& address, int port, bool local) : sockfd(-1), bootstrap(false) { 
    // Convert URL to IP (in case it happens to be a url)
    std::string ip_address = neroshop::ip::resolve(address);
    // Generate a random node ID - use public ip address for uniqueness
    public_ip_address = (local) ? get_public_ip_address() : ip_address;
    id = generate_node_id(public_ip_address, port);
    // TODO: maybe shorten the sha3-256 256-bit (32 bytes or 64 hex characters) node id to a 160-bit (20 bytes or 40 hex characters) node id to conform to the 160-bit requirement of Kademlia or not?
    //---------------------------------------------------------------------------
    memset(&storage, 0, sizeof(storage));
    if(is_ipv4(ip_address)) storage.ss_family = AF_INET;
    if(is_ipv6(ip_address)) storage.ss_family = AF_INET6;
    ////std::cout << "Socket IP Type: " << ((storage.ss_family == AF_INET6) ? "IPv6" : "IPv4\n");
    //---------------------------------------------------------------------------
    
    /*server = std::make_unique<Server>(SocketType::Socket_UDP);//(ip_address, port, SocketType::Socket_UDP);
    if(!server.bind(address, port)) {
	    throw std::runtime_error("Failed to bound to port");
	}
	std::cout << "bound to port " << port << "\n";//std::cout << NEROMON_TAG "\033[1;97mServer " + "(TCP)" + " bound to port " + std::to_string(port) + "\033[0m\n";
	
	
	if(!listen()) {
	    throw std::runtime_error("Failed to listen for connection");
	}*/
    //---------------------------------------------------------------------------
    // If this node is a local node that you own
    if(local == true) {
        // Create a UDP socket
        sockfd = socket(storage.ss_family, SOCK_DGRAM, 0);
        if(sockfd < 0) {
            perror("socket");
            throw std::runtime_error("::socket failed");
        }

        // Set socket options with setsockopt
        /*int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));*/
        // Set to broadcast mode - will broadcast a message to other nodes within the local network/to all devices on the local network, but it does not send the message beyond the network
        // This is good for testing multiple local nodes
        // In general, the first node or bootstrap node is the one that initiates the network and starts the communication. It can use broadcasting to announce its presence and make itself discoverable to other nodes. Once other nodes join the network, they can use other means, such as peer-to-peer discovery or querying a directory service, to find other nodes.
        /*if(is_bootstrap_node()) {
            int enable_broadcast = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enable_broadcast, sizeof(enable_broadcast));
        }*/

        // set a timeout of TIMEOUT_VALUE seconds for recvfrom
        struct timeval tv;
        tv.tv_sec = TIMEOUT_VALUE;  // timeout in seconds
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
}

/*neroshop::Node::Node(const Node& other)
    : id(other.id),
      version(other.version),
      data(other.data),
      info_hash_peers(other.info_hash_peers),
      //server(nullptr),
      sockfd(other.sockfd),
      sockin(other.sockin),
      sockin6(other.sockin6),
      storage(other.storage),
      routing_table(nullptr),
      public_ip_address(other.public_ip_address),
      bootstrap(other.bootstrap)
{
    //if (other.server)
    //    server = std::make_unique<Server>(*other.server);

    if (other.routing_table)
        routing_table = std::make_unique<RoutingTable>(*other.routing_table);
}*/


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
      bootstrap(other.bootstrap)
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

/*neroshop::Node& neroshop::Node::operator=(const Node& other)
{
    if (this != &other) {
        id = other.id;
        version = other.version;
        data = other.data;
        info_hash_peers = other.info_hash_peers;
        sockfd = other.sockfd;
        sockin = other.sockin;
        sockin6 = other.sockin6;
        storage = other.storage;
        public_ip_address = other.public_ip_address;
        bootstrap = other.bootstrap;

        //if (other.server)
        //    server = std::make_unique<Server>(*other.server);
        //else
        //    server.reset();

        if (other.routing_table)
            routing_table = std::make_unique<RoutingTable>(*other.routing_table);
        else
            routing_table.reset();
    }

    return *this;
}*/

//-----------------------------------------------------------------------------

std::string neroshop::Node::generate_node_id(const std::string& address, int port) {
    // TODO: increase randomness by using a hardware identifier while maintaining a stable node id
    std::string node_info = address + ":" + std::to_string(port);
    std::string hash = neroshop_crypto::sha3_256(node_info);
    return hash.substr(0, NUM_BITS / 4);
}

//-----------------------------------------------------------------------------

std::vector<neroshop::Node*> neroshop::Node::lookup(const std::string& key) {
    // Perform iterative lookup to find nodes or peers based on the key
    
    // Start by finding the closest nodes to the key in the local routing table
    std::vector<Node*> closest_nodes = find_node(key);
    
    // Perform iterative lookup to refine the search and find more nodes or peers
    // Repeat the process until the desired number of nodes or peers is found or a termination condition is met
    /*
    std::set<std::string> queried_nodes; // To keep track of already queried nodes
    const int MAX_QUERIES = 3; // Maximum number of queries per node

    // Repeat until a sufficient number of nodes are found or a stopping condition is met
    while (!closest_nodes.empty() && queried_nodes.size() < MAX_QUERIES) {
        std::vector<Node*> queried;
        for (Node* node : closest_nodes) {
            if (queried_nodes.find(node->get_id()) != queried_nodes.end()) {
                // Skip nodes that have already been queried
                continue;
            }
            queried.push_back(node);
            queried_nodes.insert(node->get_id());

            // Send find_node requests to the node
            send_find_node(node->get_id(), node->get_ip_address(), node->get_port());
        }

        // Wait for responses and update routing table
        for (Node* node : queried) {
            std::vector<Node*> response_nodes = receive_find_node_response(node->get_id());
            for (Node* response_node : response_nodes) {
                routing_table->add_node(std::unique_ptr<Node>(response_node));
            }
        }

        // Get the updated closest nodes
        closest_nodes = find_node(target_id);
    }    
    */
    // Return the list of nodes or peers found during the lookup
    return closest_nodes;
}

// Define the list of bootstrap nodes
std::vector<neroshop::Peer> bootstrap_nodes = {
    {"127.0.0.1", DEFAULT_PORT},
    {"node.neroshop.org", DEFAULT_PORT}, // $ ping neroshop.org # or nslookup neroshop.org
};

void neroshop::Node::join() {
    if(sockfd < 0) throw std::runtime_error("socket is dead");

    // Bootstrap the DHT node with a set of known nodes
    for (const auto& bootstrap_node : bootstrap_nodes) {
        std::cout << "Joining bootstrap node - " << bootstrap_node.address << ":" << bootstrap_node.port << "\n";

        // Ping each known node to confirm that it is online - the main bootstrapping primitive. If a node replies, and if there is space in the routing table, it will be inserted.
        if(!ping(bootstrap_node.address, bootstrap_node.port)) {
            std::cerr << "ping: failed to ping bootstrap node\n"; continue;
        }
        
        // Add the bootstrap node to routing table (optional) - stores the node in the routing table for later use.
        auto new_node = std::make_unique<Node>((bootstrap_node.address == "127.0.0.1") ? this->public_ip_address : bootstrap_node.address, bootstrap_node.port, false);
        new_node->set_bootstrap(true);
        Node& new_node_ref = *new_node; // take a reference to the Node object (to avoid segfault)
        routing_table->add_node(std::move(new_node)); // new_node becomes invalid after we move ownership to routing table so it cannot be used
        
        // Send a "find_node" message to the bootstrap node and wait for a response message
        auto nodes = send_find_node(this->id, (bootstrap_node.address == "127.0.0.1") ? "127.0.0.1" : new_node_ref.get_ip_address(), new_node_ref.get_port());//std::cout << "Sending find_node message to " << new_node_ref.get_ip_address() << ":" << new_node_ref.get_port() << "\n";
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
    std::cout << "Join DONE\n";
}

bool neroshop::Node::ping(const std::string& address, int port) {
    // In a DHT network, new nodes usually ping a known bootstrap node to join the network. The bootstrap node is typically a well-known node in the network that is stable and has a high probability of being online. When a new node pings the bootstrap node, it receives information about other nodes in the network and can start building its routing table.
    // Existing nodes usually ping the nodes closest to them in the keyspace to update their routing tables and ensure they are still live and responsive. In a distributed hash table, the closest nodes are determined using the XOR metric on the node IDs.
    return send_ping(address, port);
}

std::vector<neroshop::Node*> neroshop::Node::find_node(const std::string& target_id) const { 
    if(!routing_table.get()) {
        return {};
    }
    // Get the nodes from the routing table that are closest to the target node
    std::vector<Node*> closest_nodes = routing_table->find_closest_nodes(target_id);
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
        std::vector<Node*> nodes = find_node(info_hash);
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

void neroshop::Node::put(const std::string& key, const std::string& value) {    
    data[key] = value;
    ////return (data.count(key) > 0); // In case we want to return the result of the insertion
}

std::string neroshop::Node::get(const std::string& key) const {    
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "";
}

void neroshop::Node::remove(const std::string& key) {
    data.erase(key);
    ////return (data.count(key) == 0); // In case we want to return the result of the removal
}

//-------------------------------------------------------------------------------------

std::vector<uint8_t> neroshop::Node::send_query(const std::string& address, uint16_t port, const std::vector<uint8_t>& message) {
    // Step 2: Resolve the hostname and construct a destination address
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // use IPv4
    hints.ai_socktype = SOCK_DGRAM; // use UDP //SOCK_STREAM; // use TCP
    if (getaddrinfo(address.c_str(), std::to_string(port ? port : DEFAULT_PORT).c_str(), &hints, &res) != 0) {
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
    timeout.tv_sec = 2;  // Timeout in seconds
    timeout.tv_usec = 0;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt");
        close(socket_fd);
        return {};
    }
    //--------------------------------------------
    // Step 4: Receive the pong message from the server
    std::vector<uint8_t> receive_buffer(4096);
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
    // Step 1: Create the ping message
    std::string transaction_id = msgpack::generate_transaction_id();
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "ping";
    query_object["args"]["id"] = this->id;
    if(address == "127.0.0.1") {
        query_object["args"]["ephemeral_port"] = get_port(); // for testing on local network
    }
    query_object["version"] = std::string(NEROSHOP_VERSION);
    
    auto ping_message = nlohmann::json::to_msgpack(query_object); // ping_object
    //--------------------------------------------
    auto receive_buffer = send_query(address, port, ping_message);
    //--------------------------------------------
    // Step 5: Parse the pong message and extract the transaction ID and response fields
    nlohmann::json pong_message;
    try {
        pong_message = nlohmann::json::from_msgpack(receive_buffer);
    } catch (const std::exception& e) {
        std::cerr << "Node \033[91m" << address << ":" << port << "\033[0m did not respond" << std::endl;
        return false;
    }
    // Check if the parsed JSON object contains a response or an error field
    if (!pong_message.contains("response") || pong_message.contains("error")) {
        return false;
    }
    std::cout << "\033[32m" << pong_message.dump() << "\033[0m\n";
    std::string received_transaction_id = pong_message["tid"].get<std::string>();
    auto response_object = pong_message["response"];
    std::string response_id = response_object["id"].get<std::string>();
    std::string response_message = response_object["message"].get<std::string>();
    
    // Check that the pong message corresponds to the ping message
    if (received_transaction_id != transaction_id) {//assert(received_transaction_id == transaction_id && "Transaction IDs do not match");
        std::cerr << "Received pong message with incorrect transaction ID" << std::endl;
        return false;
    }
    
    // Check that the pong message is a valid response to a ping query
    if (response_message != "pong") {
        std::cerr << "Received unexpected response: " << response_message << std::endl;
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
    query_object["version"] = std::string(NEROSHOP_VERSION);
    
    auto find_node_message = nlohmann::json::to_msgpack(query_object);
    //---------------------------------------------------------
    auto receive_buffer = send_query(address, port, find_node_message);
    //---------------------------------------------------------
    // Parse nodes message
    nlohmann::json nodes_message = nlohmann::json::from_msgpack(receive_buffer);
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
    query_object["version"] = std::string(NEROSHOP_VERSION);
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
    query_object["version"] = std::string(NEROSHOP_VERSION);
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

void neroshop::Node::send_put(const std::string& key, const std::string& value) {
    
    std::string transaction_id = msgpack::generate_transaction_id();
    
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "put";////query_object["method"] = "put";
    query_object["args"]["id"] = this->id;
    query_object["args"]["key"] = key;
    query_object["args"]["value"] = value;
    
    std::vector<uint8_t> put_message = nlohmann::json::to_msgpack(query_object);
    //-----------------------------------------------
    // socket sendto and recvfrom here
    //auto receive_buffer = send_query(address, port, put_message);
    //-----------------------------------------------
    // process the response here
    // ...    
    //-----------------------------------------------
    // get result of put request
    // ...
}

void neroshop::Node::send_get(const std::string& key) {
    
    std::string transaction_id = msgpack::generate_transaction_id();
    
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "get";
    query_object["args"]["id"] = this->id;
    query_object["args"]["key"] = key;
    
    std::vector<uint8_t> get_message = nlohmann::json::to_msgpack(query_object);
    //-----------------------------------------------
    // socket sendto and recvfrom here
    //auto receive_buffer = send_query(address, port, get_message);
    //-----------------------------------------------
    // process the response here
    // ...
    //-----------------------------------------------
    // get 'value' from get request
    // ...
    
}

void neroshop::Node::send_remove(const std::string& key) {
    std::string transaction_id = msgpack::generate_transaction_id();
    
    nlohmann::json query_object;
    query_object["tid"] = transaction_id;
    query_object["query"] = "remove";////query_object["method"] = "remove";
    query_object["args"]["key"] = key;
    
    //std::string _message = bencode::encode(query);
}

//-----------------------------------------------------------------------------

void neroshop::Node::handle_request(int sockfd, struct sockaddr_in client_addr, std::vector<uint8_t> buffer, socklen_t client_addr_len, Node* node) {
    // Process the message
    std::vector<uint8_t> response = neroshop::msgpack::process(buffer, *node);

    // Send the response
    int bytes_sent = sendto(sockfd/*server.get_socket()*/, response.data(), response.size(), 0,
                                (struct sockaddr*)&client_addr, client_addr_len);
    if (bytes_sent < 0) {
        perror("sendto");
    }
        
    // Add the node that pinged this node to the routing table
    if (buffer.size() > 0) {
        nlohmann::json message = nlohmann::json::from_msgpack(buffer);
        if (message.contains("query") && message["query"] == "ping") {
            std::string sender_id = message["args"]["id"].get<std::string>();
            std::string sender_ip = inet_ntoa(client_addr.sin_addr);
            uint16_t sender_port = (message["args"].contains("ephemeral_port")) ? (uint16_t)message["args"]["ephemeral_port"] : DEFAULT_PORT;//ntohs(client_addr.sin_port);// ephemeral ports are for multiple local nodes running on the same local network
            auto node_that_pinged = std::make_unique<Node>((sender_ip == "127.0.0.1") ? node->get_public_ip_address() : sender_ip, sender_port, false); // ALWAYS use public ip so that unique id is generated and actual address is stored in routing table for others to locate your node
            node->routing_table->add_node(std::move(node_that_pinged));
            node->routing_table->print_table();
        }
    }
}

void neroshop::Node::run() {
    //if(!bootstrap) {
        run_optimized();
        return; 
    //}
    
    while (true) {
        std::vector<uint8_t> buffer(4096);
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                      (struct sockaddr*)&client_addr, &client_addr_len);
        if (bytes_received == -1 && errno == EAGAIN) {
            // No data available, continue loop
            continue;//std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait for 100 ms before retrying
        }
        else if (bytes_received < 0) {
            perror("recvfrom");
        }

        // Resize the buffer to the actual number of received bytes
        buffer.resize(bytes_received);

        /*// Process the message
        std::vector<uint8_t> response = neroshop::msgpack::process(buffer, *this);

        // Send the response
        int bytes_sent = sendto(sockfd, response.data(), response.size(), 0,
                                (struct sockaddr*)&client_addr, client_addr_len);
        if (bytes_sent < 0) {
            perror("sendto");
        }
        
        // Add the node that pinged this node to the routing table
        if (buffer.size() > 0) {
            nlohmann::json message = nlohmann::json::from_msgpack(buffer);
            if (message.contains("query") && message["query"] == "ping") {
                std::string sender_id = message["args"]["id"].get<std::string>();
                std::string sender_ip = inet_ntoa(client_addr.sin_addr);
                uint16_t sender_port = (message["args"].contains("ephemeral_port")) ? (uint16_t)message["args"]["ephemeral_port"] : DEFAULT_PORT;//ntohs(client_addr.sin_port);// ephemeral ports are for multiple local nodes running on the same local network
                auto node_that_pinged = std::make_unique<Node>((sender_ip == "127.0.0.1") ? get_public_ip_address() : sender_ip, sender_port, false); // ALWAYS use public ip so that unique id is generated and actual address is stored in routing table for others to locate your node
                routing_table->add_node(std::move(node_that_pinged));
                routing_table->print_table();
            }
        }*/
        // Create a new thread to handle the request
        std::thread request_thread([=]() { handle_request(sockfd, client_addr, buffer, client_addr_len, this); });
        request_thread.detach();
    }             
}

// This uses less CPU
void neroshop::Node::run_optimized() {
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
            std::vector<uint8_t> buffer(4096);
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

                // Create a new thread to handle the request
                std::thread request_thread([=]() { handle_request(sockfd, client_addr, buffer, client_addr_len, this); });
                request_thread.detach();
            }
        }
    }             
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

//-----------------------------------------------------------------------------

bool neroshop::Node::is_bootstrap_node() {
    return (bootstrap == true);
}

//-----------------------------------------------------------------------------

void neroshop::Node::set_bootstrap(bool bootstrap) {
    this->bootstrap = bootstrap;
}
/*int main() {
    // Create a new DHT instance and join the bootstrap node
    neroshop::Node dht_node("127.0.0.1", DEFAULT_PORT);
    neroshop::Peer bootstrap_peer = {"bootstrap.example.com", 5678}; // can be a randomly chosen existing node that provides the initial information to the new node that connects to it
    dht_node.join(bootstrap_peer);
    // Add some key-value pairs to the DHT
    dht_node.put("key1", "value1");
    dht_node.put("key2", "value2");
    // Retrieve a value from the DHT
    std::string value = dht_node.get("key1");
    std::cout << "Value: " << value << std::endl;
    // Remove a key-value pair from the DHT
    dht_node.remove("key2");
    // Find a node
    node.find_node("target_id");
    return 0;
} // g++ node.cpp ../../../crypto/sha3.cpp ../../../util/logger.cpp -I"../../../crypto/" -o node -lcrypto -lssl
*/
