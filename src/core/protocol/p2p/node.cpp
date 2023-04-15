#include "node.hpp"

#include "kademlia.hpp"
#include "../../crypto/sha3.hpp"
#include "routing_table.hpp"
#include "../../protocol/transport/ip_address.hpp"

#include <cstring> // memset
#include <future>
#include <iomanip> // std::set*

namespace neroshop_crypto = neroshop::crypto;

neroshop::Node::Node(const std::string& address, int port, bool local) : sockfd(-1), sockfd6(-1) { 
    // Convert URL to IP (in case it happens to be a url)
    std::string ip_address = url_to_ip(address);
    // Generate a random node ID - use public ip address for uniqueness
    public_ip_address = (local) ? get_public_ip_address() : ip_address;
    id = get_node_id(public_ip_address, port);
    //---------------------------------------------------------------------------
    // If this node is a local node that you own
    if(local == true) {
        std::vector<unsigned char> node_id_bytes(id.begin(), id.end());//std::vector<unsigned char> node_id_bytes(32);
        if(dht_random_bytes(node_id_bytes.data(), node_id_bytes.size()) < 0) {
            throw std::runtime_error("Failed to generate DHT node ID.");
        }    
        // Initialize UDP server
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error("::socket failed");
        }

        // Set socket options with setsockopt (for IPv6)

        // To avoid "bind" error and allow multiple nodes to run on the same IP address, we can use different port numbers for each local node we create
        // Bind to port
        int port_dynamic = port; // initial port number
        const int MAX_PORT_NUM = 65535; // port cannot go past this number
        while(true) {        
            sockin = {0};//memset(&sockin, 0, sizeof(sockin)); // both approaches are valid, but memset is more reliable and portable
            sockin.sin_family = AF_INET;
            sockin.sin_port = htons(port_dynamic);//htons(std::stoi(std::to_string(port_dynamic))); // the second approach may be useful in cases where the port number is being manipulated as a string or integer elsewhere in the code
            inet_pton(AF_INET, ip_address.c_str(), &sockin.sin_addr);//sockin.sin_addr.s_addr = htonl(INADDR_ANY) - binds to any available network interface // inet_addr(ip_address.c_str()) - binds to a specific ip // recommended to use inet_pton() over inet_addr() when working with networking in modern systems.
            
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
        if (dht_init(sockfd, 0, node_id_bytes.data(), version_bytes.data()) < 0) { // TODO: replace the zero (0) with sockfd6 (the IPv6 socket)
            perror("dht_init");
            throw std::runtime_error("Failed to initialize DHT node.");
        }
    }
    //---------------------------------------------------------------------------
    // If this is an external node that you do not own
    if(local == false) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            perror("socket");
        }
        // This socket is not meant to be bound to any port nor initialized. It will only be used to retrieve the IP address and port number
    
        memset(&sockin, 0, sizeof(sockin));
        sockin.sin_family = AF_INET;
        sockin.sin_port = htons(port);
        if(inet_pton(AF_INET, ip_address.c_str(), &sockin.sin_addr) <= 0) { //  If it returns 0, it means that the input string was not in a valid format for the specified address family
            perror("inet_pton");
        }

        // Set socket options, such as timeout or buffer size, if needed
    }
}

neroshop::Node::~Node() {
    dht_uninit();
    close(sockfd);
}

std::string neroshop::Node::get_node_id(const std::string& address, int port) {
    // Convert URL to IP
    std::string ip_address = url_to_ip(address); // or should I just use the hostname (url) instead?
    // TODO: increase randomness by using a hardware identifier while maintaining a stable node id
    std::string node_info = ip_address + ":" + std::to_string(port);
    std::string hash = neroshop_crypto::sha3_256(node_info);
    return hash.substr(0, NUM_BITS / 4);
}

// Define the list of bootstrap nodes
std::vector<neroshop::Peer> bootstrap_nodes = {
    ////{"node.neroshop.org", DEFAULT_PORT}, // $ ping neroshop.org # or nslookup neroshop.org
    {"router.bittorrent.com", DEFAULT_PORT}, 
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

        // Ping each known node to update the routing table ; dht_ping_node - the main bootstrapping primitive. If a node replies, and if there is space in the routing table, it will be inserted.
        if(!ping(bootstrap_node.address, bootstrap_node.port)) {
            std::cerr << "Failed to ping bootstrap node\n"; continue;
        }
        
        // Add the bootstrap node to routing table ; dht_insert_node - stores the node in the routing table for later use.
        auto new_node = std::make_unique<Node>(bootstrap_node.address, bootstrap_node.port, false);
        routing_table->add_node(std::move(new_node).get());
        dht_dump_tables(stdout);
        // Send a find_node message to the bootstrap node
        /*if(!find_node(bootstrap_node.address, bootstrap_node.port, this->id)) {
            std::cerr << "Failed to send find_node message to bootstrap node\n"; return;
        }*/
        //send_find_node(new_node->get_address(), new_node->get_port());
    }
    
    // Print the contents of the routing table
    routing_table->print_table();
    
    // Try listening for a connection
    // Edit: UDP socket cannot listen for connections in the same way that a TCP socket can.
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
    if(!find_node(bootstrap_node.address, bootstrap_node.port, this->id, nodes)) {
        std::cerr << "Failed to send find_node message to bootstrap node\n"; return;
    }

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
    
    struct sockaddr_storage node_addr; // sockaddr_storage can hold both IPv4 and IPv6 address unlike `sockaddr_in` and `sockaddr_in6`
    if(!create_sockaddr(address, port, node_addr)) {
        return false;
    }

    // Ping the node
    if(dht_ping_node(reinterpret_cast<const struct sockaddr*>(&node_addr), sizeof(node_addr)) < 0) {
        perror("dht_ping_node");
        return false;
    }
    
    return true;
}

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
    std::string ip_address = inet_ntoa(sockin.sin_addr);
    return ip_address;
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
