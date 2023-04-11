#include "node.hpp"

#include "kademlia.hpp"
#include "../../crypto/sha3.hpp"
#include "routing_table.hpp"

#include <cstring> // memset
#include <sys/socket.h>
#include <netinet/in.h>

namespace neroshop_crypto = neroshop::crypto;

neroshop::Node::Node(const std::string& address, int port) : sockfd(0) { 
    // Generate a random node ID
    node_id = get_node_id(address, port);
    std::vector<unsigned char> node_id_bytes(node_id.begin(), node_id.end());
    if(dht_random_bytes(node_id_bytes.data(), node_id_bytes.size()) < 0) {
        throw std::runtime_error("Failed to generate DHT node ID.");
    }    
    // Initialize UDP server
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("::socket failed");
    }

    // To avoid "bind" error and allow multiple nodes to run on the same IP address, we can use different port numbers for each local node we create
    // Bind to port
    int port_dynamic = port; // initial port number
    const int MAX_PORT_NUM = 65535; // port cannot go past this number
    while(true) {        
        sockin = {0};//memset(&sockin, 0, sizeof(sockin)); // both approaches are valid, but memset is more reliable and portable
        sockin.sin_family = AF_INET;
        sockin.sin_addr.s_addr = inet_addr(address.c_str()); // htonl(INADDR_ANY) - binds to any available network interface // inet_addr(ip) - binds to a specific ip address
        sockin.sin_port = htons(port_dynamic);//htons(std::stoi(std::to_string(port_dynamic))); // the second approach may be useful in cases where the port number is being manipulated as a string or integer elsewhere in the code
    
        if(bind(sockfd, (struct sockaddr *)&sockin, sizeof(sockin)) == 0) {
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

    // Get the port number and initialize DHT node
    version = "0.1.0";
    std::vector<unsigned char> version_bytes(version.begin(), version.end());
    if (dht_init(sockfd, 0, node_id_bytes.data(), version_bytes.data()) < 0) {
        throw std::runtime_error("Failed to initialize DHT node.");
    }
}

neroshop::Node::Node(const Peer& peer) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        // handle error
    }
    // This socket is not meant to be bound to any port. It will only be used to retrieve the IP address and port number
    
    memset(&sockin, 0, sizeof(sockin));
    sockin.sin_family = AF_INET;
    sockin.sin_port = htons(peer.port);
    if(inet_pton(AF_INET, peer.address.c_str(), &sockin.sin_addr) <= 0) {
        // handle error
    }

    // Set socket options, such as timeout or buffer size, if needed
}

neroshop::Node::~Node() {
    dht_uninit();
    close(sockfd);
}

std::string neroshop::Node::get_node_id(const std::string& address, int port) {
    // TODO: increase randomness by using a hardware identifier while maintaining a stable node id
    std::string node_info = address + ":" + std::to_string(port);
    std::string hash = neroshop_crypto::sha3_256(node_info);
    return hash.substr(0, NUM_BITS / 4);
}

// Define the list of bootstrap nodes
namespace {
std::vector<neroshop::Peer> bootstrap_nodes = {
    {"node.neroshop.org", DEFAULT_PORT}, // $ ping neroshop.org # or nslookup neroshop.org
};
}

void neroshop::Node::join() {
    // Create the routing table with the vector of nodes
    if(!routing_table.get()) {
        routing_table = std::make_unique<RoutingTable>(std::vector<Node*>{});
    }
    // Add your local node to the routing table
    routing_table->add_node(this);
    if (dht_insert_node(reinterpret_cast<const unsigned char*>(this->node_id.c_str()), (struct sockaddr *)&this->sockin, sizeof(this->sockin)) < 0) {
        perror("dht_insert_node");
        throw std::runtime_error("Failed to insert node");
    }
      
    // Bootstrap the DHT node with a set of known nodes
    // Add the bootstrap nodes to the routing table
    for (const auto& bootstrap_node : bootstrap_nodes) {
        std::cout << "joining bootstrap node - " << bootstrap_node.address << ":" << bootstrap_node.port << "\n";
        
        // Ping each known node to update the routing table ; dht_ping_node - the main bootstrapping primitive. If a node replies, and if there is space in the routing table, it will be inserted.
        if(!ping(bootstrap_node.address, bootstrap_node.port)) {
            std::cerr << "Failed to ping bootstrap node\n"; return;
        }
        
        // Add the bootstrap node to routing table ; dht_insert_node - stores the node in the routing table for later use.
        auto new_node = std::make_unique<Node>(bootstrap_node);//(bootstrap_node.address, bootstrap_node.port, false);
        routing_table->add_node(std::move(new_node).get());
        if (dht_insert_node(reinterpret_cast<const unsigned char*>(new_node->node_id.c_str()), (struct sockaddr *)&new_node->sockin, sizeof(new_node->sockin)) < 0) {
            perror("dht_insert_node");
            throw std::runtime_error("Failed to insert node");
        }
        
        // Send a find_node message to the bootstrap node
        /*if(!find_node(bootstrap_node.address, bootstrap_node.port, this->node_id)) {
            std::cerr << "Failed to send find_node message to bootstrap node\n"; return;
        }*/
        //send_find_node(new_node->get_address(), new_node->get_port());
    }
    
    // Print the contents of the routing table
    routing_table->print_table();
}

void neroshop::Node::put(const std::string& key, const std::string& value) {
    data[key] = value;
}

std::string neroshop::Node::get(const std::string& key) {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "";
}

void neroshop::Node::remove(const std::string& key) {
    data.erase(key);
}

std::vector<neroshop::Node*> find_node(const std::string& target_id) {
    /*std::vector<Node*> nodes;
    
    // Get the nodes from the routing table that are closest to the target node
    std::vector<Node*> closest_nodes = routing_table->get_closest_nodes(target, bucket_size);
    
    // Send a "find_node" message to each of the closest nodes and add any new nodes to the routing table
    for (const auto& node : closest_nodes) {
        std::cout << "finding node - " << node->get_ip_address() << ":" << node->get_port() << "\n";
        
        // Send a "find_node" message to the node
        std::vector<Node*> new_nodes = dht_find_node(reinterpret_cast<const unsigned char*>(target.c_str()), (struct sockaddr *)&(node->sockin), sizeof(node->sockin));
        
        // Add any new nodes to the routing table
        for (const auto& new_node : new_nodes) {
            if (routing_table->add_node(new_node)) {
                nodes.push_back(new_node);
            }
        }
    }
    
    return nodes;*/
    
    // Send a find_node message to the bootstrap node
    /*std::vector<Node*> nodes;
    if(!find_node(bootstrap_node.address, bootstrap_node.port, this->node_id, nodes)) {
        std::cerr << "Failed to send find_node message to bootstrap node\n"; return;
    }

    // Add the nodes returned by the find_node message to the routing table
    for (auto node : nodes) {
        if (node->node_id != this->node_id) { // don't add yourself to the routing table
            routing_table->add_node(node);
            if (dht_insert_node(reinterpret_cast<const unsigned char*>(node->node_id.c_str()), (struct sockaddr *)&node->sockin, sizeof(node->sockin)) < 0) {
                perror("dht_insert_node");
                throw std::runtime_error("Failed to insert node");
            }
        }
    }*/    
}

// ?
std::vector<std::string> neroshop::Node::get_nodes() {
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
}

bool neroshop::Node::ping(const std::string& address, int port) {
    // In a DHT network, new nodes usually ping a known bootstrap node to join the network. The bootstrap node is typically a well-known node in the network that is stable and has a high probability of being online. When a new node pings the bootstrap node, it receives information about other nodes in the network and can start building its routing table.
    // Existing nodes usually ping the nodes closest to them in the keyspace to update their routing tables and ensure they are still live and responsive. In a distributed hash table, the closest nodes are determined using the XOR metric on the node IDs.

    // Get the IP address and port of the node we want to ping
    struct sockaddr_in node_addr;
    memset(&node_addr, 0, sizeof(node_addr));
    node_addr.sin_family = AF_INET;
    node_addr.sin_port = htons(port); // port goes here
    inet_pton(AF_INET, address.c_str(), &node_addr.sin_addr); // IP address goes here // inet_pton (short for "presentation to network") is a more modern and versatile function than inet_addr and can handle both IPv4 and IPv6 addresses, making it a more flexible and future-proof option
    
    // Ping the node
    if(dht_ping_node(reinterpret_cast<const struct sockaddr*>(&node_addr), sizeof(node_addr)) < 0) {
        perror("dht_ping_node");
        std::cerr << "Failed to ping node." << "\n";
        return false;
    }
    return true;
}

void neroshop::Node::loop() {
        struct sockaddr_storage sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        unsigned char buffer[1024];

        // Wait for incoming packets
        int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender_addr, &sender_len);
        if (bytes_received < 0) {
            // Handle error
            //continue;
        }

        // Parse incoming packet
        int event = 0;  // Set the event to 0 for responses
        const unsigned char* info_hash = NULL;  // Set the info hash to NULL for responses
        void* data = buffer;
        size_t data_len = bytes_received;


    time_t tosleep = 0;
    if (dht_periodic(NULL, 0, NULL, 0, &tosleep, NULL, NULL) < 0) {
        perror("dht_periodic");
        throw std::runtime_error("Periodic failed");
    }
    if (tosleep > 0) {
        sleep(tosleep);
    }
    
        // Send response
        unsigned char response[1024];
        int response_len = 0;
        /*int result = sendto((struct sockaddr*)&sender_addr, sender_len, event, info_hash, response, sizeof(response), &response_len, buffer, bytes_received, &dht_state);
        if (result < 0) {
            // Handle error
            //continue;
        }*/

        // Send response packet
        int bytes_sent = sendto(sockfd, response, response_len, 0, (struct sockaddr*)&sender_addr, sender_len);
        if (bytes_sent < 0) {
            // Handle error
            //continue;
        }    
}

std::string neroshop::Node::get_id() {
    std::string address = get_ip_address();
    unsigned int port = get_port();
    return get_node_id(address, port);
}

std::string neroshop::Node::get_ip_address() const {
    std::string ip_address = inet_ntoa(sockin.sin_addr);
    return ip_address;
}

int neroshop::Node::get_port() const {
    uint16_t port = ntohs(sockin.sin_port);
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
