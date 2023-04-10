#include "node.hpp"

#include "kademlia.hpp"
#include "../../crypto/sha3.hpp"

#include <cstring> // memset
#include <sys/socket.h>
#include <netinet/in.h>

namespace neroshop_crypto = neroshop::crypto;

neroshop::Node::Node(const std::string& address, int port) {    
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

neroshop::Node::~Node() {
    close(sockfd);
    dht_uninit();
}

std::string neroshop::Node::get_node_id(const std::string& address, int port) {
    // This should generate a unique and deterministic node ID based on the node's address and port, which would remain constant each time the node goes online. This is a valid approach for generating stable node IDs.
    // TODO: increase randomness
    std::string node_info = address + ":" + std::to_string(port);
    std::string hash = neroshop_crypto::sha3_256(node_info);
    return hash.substr(0, NUM_BITS / 4);
}

void neroshop::Node::join(const Peer& bootstrap_peer) {
    std::cout << "joining bootstrap node - " << bootstrap_peer.address << ":" << bootstrap_peer.port << "\n";
    /*// Bootstrap the DHT node with a set of known nodes (array n)
    nodes = dht_ping_nodes(s, family, argv + 1, argc - 1, n, MAX_NODES);
    printf("Got %d bootstrap nodes\n", nodes);
    // Join the DHT network
    rc = dht_join(s, myid);
    printf("Join returned %d\n", rc);*/
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

// ?
std::vector<std::string> neroshop::Node::get_nodes() {

}

bool neroshop::Node::ping(const std::string& address, int port) {
    // In a DHT network, new nodes usually ping a known bootstrap node to join the network. The bootstrap node is typically a well-known node in the network that is stable and has a high probability of being online. When a new node pings the bootstrap node, it receives information about other nodes in the network and can start building its routing table.
    // Existing nodes usually ping the nodes closest to them in the keyspace to update their routing tables and ensure they are still live and responsive. In a distributed hash table, the closest nodes are determined using the XOR metric on the node IDs.

    // Get the IP address and port of the node we want to ping
    struct sockaddr_in node_addr;
    memset(&node_addr, 0, sizeof(node_addr));
    node_addr.sin_family = AF_INET;
    node_addr.sin_port = htons(port); // port goes here
    inet_pton(AF_INET, address.c_str(), &node_addr.sin_addr); // IP address goes here 
    
    // Ping the node
    if(dht_ping_node(reinterpret_cast<const struct sockaddr*>(&node_addr), sizeof(node_addr)) < 0) {
        std::cerr << "Failed to ping node." << "\n";
        return false;
    }
    return true;
}

// You should call this function in a loop to continuously listen for incoming packets.
void neroshop::Node::listen_for_packets(int sockfd) {
    /*while (true) {
        struct sockaddr_storage sender_addr;
        socklen_t sender_len = sizeof(sender_addr);
        unsigned char buffer[1024];

        // Wait for incoming packets
        int bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&sender_addr, &sender_len);
        if (bytes_received < 0) {
            // Handle error
            continue;
        }

        // Parse incoming packet
        int event = 0;  // Set the event to 0 for responses
        const unsigned char* info_hash = NULL;  // Set the info hash to NULL for responses
        void* data = buffer;
        size_t data_len = bytes_received;

        // Handle incoming packet
        dht_periodic(data, data_len, (struct sockaddr*)&sender_addr, sender_len, NULL, dht_callback, &dht_state);

        // Send response
        unsigned char response[1024];
        int response_len = 0;
        int result = dht_send((struct sockaddr*)&sender_addr, sender_len, event, info_hash, response, sizeof(response), &response_len, buffer, bytes_received, &dht_state);
        if (result < 0) {
            // Handle error
            continue;
        }

        // Send response packet
        int bytes_sent = sendto(sockfd, response, response_len, 0, (struct sockaddr*)&sender_addr, sender_len);
        if (bytes_sent < 0) {
            // Handle error
            continue;
        }
    }*/
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
