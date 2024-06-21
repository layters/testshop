#pragma once

#include "../transport/server.hpp" // TCP, UDP. IP-related headers here

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory> // std::unique_ptr
#include <functional> // std::function
#include <shared_mutex>

const int NUM_BITS = 256;

namespace neroshop {

class RoutingTable; // forward declaration
class KeyMapper;

struct Peer {
    std::string address;
    int port;
};

enum class NodeStatus { Inactive, Idle, Active };

class Node {
private:
    std::string id;
    std::string version;
    std::unordered_map<std::string, std::string> data; // internal hash table that stores key-value pairs 
    std::unordered_map<std::string, std::vector<Peer>> providers; // maps a data's hash (key) to a vector of Peers who have the data
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
    int check_counter; // Counter to track the number of consecutive failed checks
    mutable std::shared_mutex data_mutex;
    // Declare a mutex to protect access to the routing table
    std::shared_mutex node_read_mutex; // Shared mutex for routing table access
    std::shared_mutex node_write_mutex; // Shared mutex for routing table access
    std::unique_ptr<KeyMapper> key_mapper;
    // Generates a node id from address and port combination
    std::string generate_node_id(const std::string& address, int port);
    // Determines if node1 is closer to the target_id than node2
    bool is_closer(const std::string& target_id, const std::string& node1_id, const std::string& node2_id);
    //---------------------------------------------------
    int set(const std::string& key, const std::string& value); // Updates the value without changing the key. set cannot be accessed directly but only through put
    bool verify(const std::string& value) const;
    void expire(const std::string& key, const std::string& value); // Removes any expired data from hash table
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
        return (this->public_ip_address == other.public_ip_address && this->get_port() == other.get_port());
    }
    
    std::vector<uint8_t> send_query(const std::string& address, uint16_t port, const std::vector<uint8_t>& message, int recv_timeout = 5);
    //---------------------------------------------------
    bool send_ping(const std::string& address, int port);
    std::vector<std::unique_ptr<Node>> send_find_node(const std::string& target_id, const std::string& address, uint16_t port);
    int send_put(const std::string& key, const std::string& value);
    int send_store(const std::string& key, const std::string& value);
    std::string send_get(const std::string& key);
    std::string send_find_value(const std::string& key);
    void send_remove(const std::string& key);
    void send_map(const std::string& address, int port); // Distributes indexing data to a single node
    void send_map_v2(const std::string& address, int port);
    std::vector<Peer> send_get_providers(const std::string& data_hash);
    //---------------------------------------------------
    ////std::vector<Node*> lookup(const std::string& key); // In Kademlia, the primary purpose of the lookup function is to find the nodes responsible for storing a particular key in the DHT, rather than retrieving the actual value of the key. The lookup function helps in locating the nodes that are likely to have the key or be able to provide information about it.
    //---------------------------------------------------
    void join(/*std::function<void()> on_join_callback*/); // Sends a join message to the bootstrap peer to join the network
    void run(); // Main loop that listens for incoming messages
    void run_optimized(); // Uses less CPU than run but slower to process requests
    void periodic_check();
    void periodic_refresh(); // Periodically sends find_node queries
    void periodic_republish(); // Periodically republishes in-memory hash table data
    void periodic_purge(); // Periodically removes expired in-memory hash table data
    void refresh();
    void republish();
    bool validate(const std::string& key, const std::string& value); // Validates data before storing it
    int cache(const std::string& key, const std::string& value); // Cache data created by local client
    //---------------------------------------------------
    void persist_routing_table(const std::string& address, int port); // JIC bootstrap node faces outage and needs to recover
    void rebuild_routing_table(); // Re-builds routing table from data stored on disk
    //---------------------------------------------------
    void on_ping(const std::vector<uint8_t>& buffer, const struct sockaddr_in& client_addr);
    void on_map(const std::vector<uint8_t>& buffer, const struct sockaddr_in& client_addr);
    ////void on_dead_node(const std::vector<std::string>& node_ids);
    ////bool on_keyword_blocked(const std::string& keyword);
    ////bool on_node_blacklisted(const std::string& address);
    ////bool on_data_expired();
    //---------------------------------------------------
    // DHT Query Types
    bool ping(const std::string& address, int port); // A simple query to check if a node is online and responsive.
    std::vector<Node*> find_node(const std::string& target_id, int count) const;// override; // A query to find the contact information for a specific node in the DHT. // Finds the node closest to the target_id
    int put(const std::string& key, const std::string& value); // A query to store a value in the DHT.    // Stores the key-value pair in the DHT
    int store(const std::string& key, const std::string& value);
    std::string get(const std::string& key) const; // A query to get a specific value stored in the DHT.         // Retrieves the value associated with the key from the DHT
    std::string find_value(const std::string& key) const;
    int remove(const std::string& key); // Remove a key-value pair from the DHT
    void map(const std::string& key, const std::string& value); // Maps search terms to keys
    void add_provider(const std::string& data_hash, const Peer& peer);
    void remove_providers(const std::string& data_hash);
    void remove_provider(const std::string& data_hash, const std::string& address, int port);
    std::vector<Peer> get_providers(const std::string& data_hash) const; // A query to get a list of peers for a specific torrent or infohash.
    //---------------------------------------------------
    std::string get_id() const; // get ID of this node
    std::string get_ip_address() const;
    std::string get_local_ip_address() const;
    std::string get_device_ip_address() const;
    std::string get_public_ip_address() const;
    uint16_t get_port() const;
    RoutingTable * get_routing_table() const;
    int get_peer_count() const;
    int get_active_peer_count() const;
    int get_idle_peer_count() const;
    NodeStatus get_status() const;
    std::string get_status_as_string() const;
    std::vector<std::string> get_keys() const;
    std::vector<std::pair<std::string, std::string>> get_data() const;
    ////Server * get_server() const;
    std::string get_cached(const std::string& key);
    
    void set_bootstrap(bool bootstrap);
    
    bool is_bootstrap_node() const;
    bool is_hardcoded() const;
    static bool is_hardcoded(const std::string& address, uint16_t port);
    bool has_key(const std::string& key) const;
    bool has_key_cached(const std::string& key) const;
    bool has_value(const std::string& value) const;
    bool is_dead() const;
};

}
