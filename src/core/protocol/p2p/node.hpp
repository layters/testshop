#pragma once

#include "../../server.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory> // std::unique_ptr

#define DEFAULT_PORT 6881
#define MAX_NODES 100 // the maximum number of nodes that can be stored in the DHT routing table

const int NUM_BITS = 256;
const int NUM_PEERS = 10;

namespace neroshop {

struct Peer {
    std::string address;
    int port;
};

class Node {
private:
    std::string node_id;
    std::string version;
    std::unordered_map<std::string, Peer> peers;
    std::unordered_map<std::string, std::string> data; // internal hash table that stores key-value pairs 
    int sockfd;//std::unique_ptr<Server> server;// a node acts as a server so it should have a server object
    struct sockaddr_in sockin;

    std::string get_node_id(const std::string& address, int port);
    // Determines if node1 is closer to the target_id than node2
    bool is_closer(const std::string& target_id, const std::string& node1_id, const std::string& node2_id);// {
    // Returns the closest peers to the target_id
    std::vector<Peer> get_closest_peers(const std::string& target_id);
public:
    Node(const std::string& address, int port);
    ~Node();
    // Sends a join message to the bootstrap peer to join the network
    void join(const Peer& bootstrap_peer);
    // Stores the key-value pair in the DHT
    void put(const std::string& key, const std::string& value);
    // Retrieves the value associated with the key from the DHT
    std::string get(const std::string& key);
    // Remove a key-value pair from the DHT
    void remove(const std::string& key);    
    // Finds the node closest to the target_id
    void find_node(const std::string& target_id);
    // DHT Wrapper functions
    std::vector<std::string> get_nodes();
    bool ping(const std::string& address, int port);
    void listen_for_packets(int sockfd);
};
/*
This is just a basic example implementation of a DHT in C++, and there are many details that are left out, such as the message routing mechanism, the peer discovery algorithm, and the node join and leave protocols. The actual implementation of a DHT can be quite complex and involves many different components, so this code should be considered as a starting point for further development.
*/
}
