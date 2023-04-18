#include "node.hpp"

#include "kademlia.hpp"
#include "../../crypto/sha3.hpp"
#include "routing_table.hpp"
#include "../transport/ip_address.hpp"
#include "../rpc/bencode.hpp"
#include "../rpc/krpc.hpp"

#include <cstring> // memset
#include <future>
#include <iomanip> // std::set*

namespace neroshop_crypto = neroshop::crypto;

neroshop::Node::Node(const std::string& address, int port, bool local) : sockfd(-1) { 
    // Convert URL to IP (in case it happens to be a url)
    std::string ip_address = url_to_ip(address);
    // Generate a random node ID - use public ip address for uniqueness
    public_ip_address = (local) ? get_public_ip_address() : ip_address;
    id = generate_node_id(public_ip_address, port);
    //---------------------------------------------------------------------------
    memset(&storage, 0, sizeof(storage));
    if(is_ipv4(ip_address)) storage.ss_family = AF_INET;
    if(is_ipv6(ip_address)) storage.ss_family = AF_INET6;
    std::cout << "Socket IP Type: " << ((storage.ss_family == AF_INET6) ? "IPv6" : "IPv4\n");
    //---------------------------------------------------------------------------
    // If this node is a local node that you own
    if(local == true) {
        // Create a UDP socket
        sockfd = socket(storage.ss_family, SOCK_DGRAM, 0);
        if(sockfd < 0) {
            perror("socket");
            throw std::runtime_error("::socket failed");
        }
        
        // Make sockfd non-blocking
        int flags = fcntl(sockfd, F_GETFL, 0);
        if(flags == -1) {
            perror("fcntl");
        }
        if(fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
            perror("fcntl");
        }

        // Set socket options with setsockopt (for IPv6)

        // To avoid "bind" error and allow multiple nodes to run on the same IP address, we can use different port numbers for each local node we create
        // Bind to port
        int port_dynamic = port; // initial port number
        const int MAX_PORT_NUM = 65535; // port cannot go past this number
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
                std::cout << "Node bound to port " << port_dynamic << std::endl;
                break;
            }
            std::cout << "Port " << port_dynamic << " already in use." << std::endl;
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
}

neroshop::Node::~Node() {
    if(sockfd > 0) {
        close(sockfd);
        sockfd = -1;
    }
}

std::string neroshop::Node::generate_node_id(const std::string& address, int port) {
    // TODO: increase randomness by using a hardware identifier while maintaining a stable node id
    std::string node_info = address + ":" + std::to_string(port);
    std::string hash = neroshop_crypto::sha3_256(node_info);
    return hash.substr(0, NUM_BITS / 4);
}

// This function uses the C++ standard library's random number generator to generate a random number between 0 and 65535 (the maximum value of a 2-byte unsigned integer). It then converts this number to a 2-byte string, with the most significant byte first, as required by the DHT protocol.
static std::string generate_transaction_id() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned short> dis(0, std::numeric_limits<unsigned short>::max());
    unsigned short random_num = dis(gen);
    char transaction_id[2];
    transaction_id[0] = static_cast<char>(random_num >> 8);
    transaction_id[1] = static_cast<char>(random_num & 0xFF);
    return std::string(transaction_id, 2);
}

// Define the list of bootstrap nodes
std::vector<neroshop::Peer> bootstrap_nodes = {
    ////{"node.neroshop.org", DEFAULT_PORT}, // $ ping neroshop.org # or nslookup neroshop.org
    {"router.bittorrent.com", DEFAULT_PORT}, 
    //{"router.utorrent.com", DEFAULT_PORT}, 
    //{"dht.transmissionbt.com", DEFAULT_PORT}, 
    //{"dht.aelitis.com", DEFAULT_PORT}, 
};

void neroshop::Node::join() {
    // Create the routing table with the vector of nodes
    if(!routing_table.get()) {
        routing_table = std::make_unique<RoutingTable>(std::vector<Node*>{});
    }
    
    // Was going to add this node to the routing table, but nodes do not typically add their own public IP address to their routing table
      
    // Bootstrap the DHT node with a set of known nodes
    for (const auto& bootstrap_node : bootstrap_nodes) {
        std::cout << "joining bootstrap node - " << bootstrap_node.address << ":" << bootstrap_node.port << "\n";

        // Ping each known node to update the routing table - the main bootstrapping primitive. If a node replies, and if there is space in the routing table, it will be inserted.
        if(!ping(bootstrap_node.address, bootstrap_node.port)) {
            std::cerr << "Failed to ping bootstrap node\n"; continue;
        }
        
        // Add the bootstrap node to routing table ; dht_insert_node - stores the node in the routing table for later use.
        auto new_node = std::make_unique<Node>(bootstrap_node.address, bootstrap_node.port, false);
        routing_table->add_node(std::move(new_node).get());
        
        // Send a "find_node" message to the bootstrap node and wait for a response message
        auto nodes = find_node(new_node->get_id());
        if(nodes.empty()) {
            std::cerr << "Failed to send find_node message to bootstrap node\n"; continue;
        }
        // Then add nodes to the routing table
    }
    
    // Print the contents of the routing table
    routing_table->print_table();
    
    // Try listening for a connection
    // Edit: UDP socket cannot listen for connections in the same way that a TCP socket can.
}

bool neroshop::Node::ping(const std::string& address, int port) {
    // In a DHT network, new nodes usually ping a known bootstrap node to join the network. The bootstrap node is typically a well-known node in the network that is stable and has a high probability of being online. When a new node pings the bootstrap node, it receives information about other nodes in the network and can start building its routing table.
    // Existing nodes usually ping the nodes closest to them in the keyspace to update their routing tables and ensure they are still live and responsive. In a distributed hash table, the closest nodes are determined using the XOR metric on the node IDs. 
    ////std::string ip_address = url_to_ip(address);
    
    std::string transaction_id = generate_transaction_id();
    std::string message_type = "q"; // q = query
    std::string query_type = "ping";

    bencode_dict ping_dict;
    ping_dict["t"] = transaction_id;
    ping_dict["y"] = message_type;
    ping_dict["q"] = query_type;

    std::string ping_message = bencode::encode(ping_dict);
    std::cout << "ping_message (sent): " << ping_message << "\n";

    // Create a sockaddr_in structure containing the IP address and port number of the destination node.
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(/*ip_*/address.c_str());
    
    // Send the ping message to the destination node using the sendto() function.
    ssize_t bytes_sent = sendto(sockfd, ping_message.c_str(), ping_message.length(), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (bytes_sent == -1) {
        std::cerr << "Error sending data: " << strerror(errno) << std::endl;
        return false;
    } else {
        std::cout << "Sent " << bytes_sent << " bytes of data" << std::endl;
    }
    
    // Receive the pong response from the destination node using the recvfrom() function.
    char buffer[1024];
    int recvlen = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    if(recvlen == -1) {
        perror("recvfrom");
        return false;
    } 
    std::string pong_message(buffer, recvlen);
    std::cout << "pong_message (received): " << pong_message << "\n";
    
    // Parse the pong response to extract the transaction ID, message type, and response fields.
    size_t size = 0;
    bencode_dict pong_dict = bencode::decode_dict(pong_message, size);
    for (const auto& [key, value] : pong_dict) {
        std::cout << key << ": ";
        if (std::holds_alternative<int64_t>(value)) {
            std::cout << std::get<int64_t>(value) << std::endl;
        } else if (std::holds_alternative<std::string>(value)) {
            std::cout << std::get<std::string>(value) << std::endl;
        } else if (std::holds_alternative<bencode_list>(value)) {
            const bencode_list& list = std::get<bencode_list>(value);
            std::cout << "[";

            for (const auto& item : list) {
                if (std::holds_alternative<int64_t>(item)) {
                    std::cout << std::get<int64_t>(item);
                } else if (std::holds_alternative<std::string>(item)) {
                    std::cout << std::get<std::string>(item);
                }
                std::cout << ", ";
            }

            std::cout << "]" << std::endl;
        }
    }

    std::string pong_transaction_id = std::get<std::string>(pong_dict["t"]);
    std::string pong_message_type = std::get<std::string>(pong_dict["y"]);
    std::string pong_response_type = std::get<std::string>(pong_dict["r"]); // r = response
    

    return true;
}

std::vector<neroshop::Node*> neroshop::Node::find_node(const std::string& target_id) {
    std::vector<Node*> nodes = {};
    
    std::string transaction_id = generate_transaction_id();

    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "find_node";
    query["a"] = bencode_list { this->id, target_id }; // "id", "target"//, "address": get_public_ip_address() + ":" + std::to_string(DEFAULT_PORT);
    
    std::string find_node_message = bencode::encode(query);
    std::cout << "find_node: " << find_node_message << "\n";
    
// Get the nodes from the routing table that are closest to the target node
    //std::vector<Node*> closest_nodes = routing_table->get_closest_nodes(target, bucket_size);
        
    // Send a find_node message to the bootstrap node
    /*std::vector<Node*> nodes;
    // ...

    // Add the nodes returned by the find_node message to the routing table
    for (auto node : nodes) {
        if (node->id != this->id) { // don't add yourself to the routing table
            routing_table->add_node(node);
            if (dht_insert_node(reinterpret_cast<const unsigned char*>(node->id.c_str()), (struct sockaddr *)&node->sockin, sizeof(node->sockin)) < 0) {
                perror("dht_insert_node");
                throw std::runtime_error("Failed to insert node");
            }
        }
    }*/
    return nodes;    
}

std::vector<neroshop::Peer> neroshop::Node::get_peers() {
    std::vector<Peer> peers = {};
    
    std::string transaction_id = generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "get_peers";
    
    std::string get_peers_message = bencode::encode(query);
    
    
    return peers;
}

void neroshop::Node::announce_peer() {
    
    std::string transaction_id = generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "announce_peer";
    
    std::string announce_peer_message = bencode::encode(query);
}

void neroshop::Node::put(const std::string& key, const std::string& value) {
    
    std::string transaction_id = generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "put";
    //query["a"] = 
    
    std::string put_message = bencode::encode(query);
    
    
    data[key] = value;
}

std::string neroshop::Node::get(const std::string& key) {
    
    std::string transaction_id = generate_transaction_id();
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = "get";
    //query["a"] = 
    
    std::string get_message = bencode::encode(query);
    
    
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "";
}

void neroshop::Node::remove(const std::string& key) {
    
    /*std::string transaction_id = generate_transaction_id();
    std::string message_type = "q";
    std::string query_type = "";
    
    bencode_dict query;
    query["t"] = transaction_id;
    query["y"] = "q";
    query["q"] = 
    
    std::string _message = bencode::encode(query);*/
    
    
    data.erase(key);
}

// ?
/*std::vector<std::string> neroshop::Node::get_nodes() {
    struct sockaddr_in sin[100];
    struct sockaddr_in6 sin6[100];
    int num = 100, num6 = 100;

    if(dht_get_nodes(sin, &num, sin6, &num6) < 0) {
        perror("dht_get_nodes returned an error");
    }

    for (int i = 0; i < num; i++) {
        char buf[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(sin[i].sin_addr), buf, INET_ADDRSTRLEN);
        printf("IPv4 node %d: %s:%d\n", i, buf, ntohs(sin[i].sin_port));
    }

    for (int i = 0; i < num6; i++) {
        char buf[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &(sin6[i].sin6_addr), buf, INET6_ADDRSTRLEN);
        printf("IPv6 node %d: [%s]:%d\n", i, buf, ntohs(sin6[i].sin6_port));
    }
}*/

void neroshop::Node::loop() {
    // Check for incoming DHT messages using dht_periodic()
    /*int tosleep = 0;
    rc = dht_periodic((unsigned char*)"mytorrent", strlen("mytorrent"),
                          (struct sockaddr*)&sin_peer, sizeof(sin_peer),
                          &tosleep, &dht_callback, NULL);
    if (rc < 0) {
        perror("dht_periodic");
        return;
    }
    
    // Sleep for a short time before running the DHT node again
    sleep(tosleep);*/
    //-----------------------------
    /*fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    // Wait for incoming data using select()
    int ret = select(sockfd + 1, &readfds, NULL, NULL, NULL);
    if (ret < 0) {
        // Handle error
    }
    
    // Receive incoming data using recvfrom()
    char buffer[1024];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                          (struct sockaddr *)&client_addr, &client_len);
    if (nbytes < 0) {
        // Handle error
    }
    
    // Process the received data as needed
    printf("Received %d bytes from %s:%d\n", nbytes,
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));*/
}

std::string neroshop::Node::get_id() {
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

int neroshop::Node::get_port() const {
    uint16_t port = -1;
    
    if(storage.ss_family == AF_INET) {
        port = ntohs(sockin.sin_port);
    } 
    if(storage.ss_family == AF_INET6) {
        port = ntohs(sockin6.sin6_port);
    }
    
    return port;
}

neroshop::RoutingTable * neroshop::Node::get_routing_table() const {
    return routing_table.get();
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
