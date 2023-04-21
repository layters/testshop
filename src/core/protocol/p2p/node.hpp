#pragma once

#include "../transport/server.hpp" // TCP, UDP. IP-related headers here

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory> // std::unique_ptr

#define DEFAULT_PORT DEFAULT_UDP_PORT
#define MAX_NODES 100 // the maximum number of nodes that can be stored in the DHT routing table
#define TIMEOUT_VALUE 5 // A reasonable timeout value for a DHT node could be between 5 to 30 seconds.

const int NUM_BITS = 256;
const int NUM_PEERS = 10;

namespace neroshop {

class RoutingTable; // forward declaration

struct Peer {
    std::string address;
    int port;
};

class Node {
private:
    std::string id;
    std::string version;
    std::unordered_map<std::string, std::string> data; // internal hash table that stores key-value pairs 
    int sockfd;//std::unique_ptr<Server> server;// a node acts as a server so it should have a server object
    struct sockaddr_in sockin; // IPV4
    struct sockaddr_in6 sockin6; // IPV6
    struct sockaddr_storage storage;
    std::unique_ptr<RoutingTable> routing_table; // Pointer to the node's routing table
    friend class RoutingTable;
    std::string public_ip_address;
    bool bootstrap;
    // Generates a node id from address and port combination
    std::string generate_node_id(const std::string& address, int port);
    // Determines if node1 is closer to the target_id than node2
    bool is_closer(const std::string& target_id, const std::string& node1_id, const std::string& node2_id);
public:
    Node(const std::string& address, int port, bool local); // Binds a socket to a port and initializes the DHT
    Node(const Peer& peer); // Creates a node/socket from a peer without binding socket or initializing the DHT
    ~Node();
    // Sends a join message to the bootstrap peer to join the network
    void join();//(const Peer& bootstrap_peer);
    // DHT Query Types
    bool ping(const std::string& address, int port); // A simple query to check if a node is online and responsive.
    std::vector<Node*> find_node(const std::string& target_id); // A query to find the contact information for a specific node in the DHT. // Finds the node closest to the target_id
    std::vector<Peer> get_peers(); // A query to get a list of peers for a specific torrent or infohash.
    void announce_peer(); // A query to announce that a peer has joined a specific torrent or infohash.
    void put(const std::string& key, const std::string& value); // A query to store a value in the DHT.    // Stores the key-value pair in the DHT
    std::string get(const std::string& key); // A query to get a specific value stored in the DHT.         // Retrieves the value associated with the key from the DHT
    // Remove a key-value pair from the DHT
    void remove(const std::string& key);
    // TODO: use std::any for `data` unordered_map value
    // Main loop that listens for connections
    void loop();
    //std::vector<std::string> get_nodes();
    std::string get_id(); // get ID of this node
    std::string get_ip_address() const;
    std::string get_local_ip_address() const;
    std::string get_device_ip_address() const;
    std::string get_public_ip_address() const;
    int get_port() const;
    RoutingTable * get_routing_table() const;
    void set_bootstrap(bool bootstrap);
    bool is_bootstrap_node();
};
/*
This is just a basic example implementation of a DHT in C++, and there are many details that are left out, such as the message routing mechanism, the peer discovery algorithm, and the node join and leave protocols. The actual implementation of a DHT can be quite complex and involves many different components, so this code should be considered as a starting point for further development.
*/
}
