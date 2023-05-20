#pragma once

//#include "dht_node.hpp"
#include "../transport/server.hpp" // TCP, UDP. IP-related headers here

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory> // std::unique_ptr
#include <functional> // std::function

#define DEFAULT_PORT DEFAULT_UDP_PORT
#define TIMEOUT_VALUE 5 // A reasonable timeout value for a DHT node could be between 5 to 30 seconds.

const int NUM_BITS = 256;
const int NUM_PEERS = 10;

namespace neroshop {

class RoutingTable; // forward declaration

struct Peer {
    std::string address;
    int port;
};

class Node {//: public DHTNode {
private:
    std::string id;
    std::string version;
    std::unordered_map<std::string, std::string> data; // internal hash table that stores key-value pairs 
    std::unordered_map<std::string, std::vector<Peer>> info_hash_peers; // maps an info_hash to a vector of Peers
    ////std::unique_ptr<Server> server;// a node acts as a server so it should have a server object
    int sockfd;
    struct sockaddr_in sockin; // IPV4
    struct sockaddr_in6 sockin6; // IPV6
    struct sockaddr_storage storage;
    std::unique_ptr<RoutingTable> routing_table; // Pointer to the node's routing table
    friend class RoutingTable;
    friend class Server;
    std::string public_ip_address;
    bool bootstrap;
    // Generates a node id from address and port combination
    std::string generate_node_id(const std::string& address, int port);
    // Determines if node1 is closer to the target_id than node2
    bool is_closer(const std::string& target_id, const std::string& node1_id, const std::string& node2_id);
public:
    Node(const std::string& address, int port, bool local); // Binds a socket to a port and initializes the DHT
    //Node(const Node& other); // Copy constructor
    Node(Node&& other) noexcept; // Move constructor
    //Node(const Peer& peer); // Creates a node/socket from a peer without binding socket or initializing the DHT
    ~Node();
    
    //Node& operator=(const Node& other);
    //Node& operator=(Node&&) noexcept;
    bool operator==(const Node& other) const {
        // compare the relevant fields of this Node and other
        return this->id == other.id;
    }
    
    std::vector<uint8_t> send_query(const std::string& address, uint16_t port, const std::vector<uint8_t>& message);
    //---------------------------------------------------
    bool send_ping(const std::string& address, int port);
    std::vector<Node*> send_find_node(const std::string& target_id, const std::string& address, uint16_t port);
    void send_get_peers(const std::string& info_hash);
    void send_announce_peer(const std::string& info_hash, int port, const std::string& token);
    void send_add_peer(const std::string& info_hash, const Peer& peer);
    void send_put(const std::string& key, const std::string& value);
    void send_get(const std::string& key);
    void send_remove(const std::string& key);
    //---------------------------------------------------
    std::vector<Node*> lookup(const std::string& key); // In Kademlia, the primary purpose of the lookup function is to find the nodes responsible for storing a particular key in the DHT, rather than retrieving the actual value of the key. The lookup function helps in locating the nodes that are likely to have the key or be able to provide information about it.
    //---------------------------------------------------    
    void join(/*std::function<void()> on_join_callback*/); // Sends a join message to the bootstrap peer to join the network
    void run(); // Main loop that listens for incoming messages
    void run_optimized(); // Uses less CPU than run but slower to process requests
    static void handle_request(int sockfd, struct sockaddr_in client_addr, std::vector<uint8_t> buffer, socklen_t client_addr_len, Node* node);
    // DHT Query Types
    bool ping(const std::string& address, int port); // A simple query to check if a node is online and responsive.
    std::vector<Node*> find_node(const std::string& target_id) const;// override; // A query to find the contact information for a specific node in the DHT. // Finds the node closest to the target_id
    std::vector<Peer> get_peers(const std::string& info_hash) const; // A query to get a list of peers for a specific torrent or infohash.
    void announce_peer(const std::string& info_hash, int port, const std::string& token); // A query to announce that a peer has joined a specific torrent or infohash.
    void add_peer(const std::string& info_hash, const Peer& peer);
    void remove_peer(const std::string& info_hash);
    void put(const std::string& key, const std::string& value); // A query to store a value in the DHT.    // Stores the key-value pair in the DHT
    std::string get(const std::string& key) const; // A query to get a specific value stored in the DHT.         // Retrieves the value associated with the key from the DHT
    void remove(const std::string& key); // Remove a key-value pair from the DHT
    //---------------------------------------------------
    std::string get_id() const; // get ID of this node
    std::string get_ip_address() const;
    std::string get_local_ip_address() const;
    std::string get_device_ip_address() const;
    std::string get_public_ip_address() const;
    uint16_t get_port() const;
    RoutingTable * get_routing_table() const;
    void set_bootstrap(bool bootstrap);
    bool is_bootstrap_node();
    ////Server * get_server() const;
};
/*
This is just a basic example implementation of a DHT in C++, and there are many details that are left out, such as the message routing mechanism, the peer discovery algorithm, and the node join and leave protocols. The actual implementation of a DHT can be quite complex and involves many different components, so this code should be considered as a starting point for further development.
*/
}
