#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory> // std::unique_ptr
#include <functional> // std::function
#include <shared_mutex>
#include <deque>
#include <mutex>
#include <future>
#include <atomic>
#include <chrono>

#include <nlohmann/json.hpp>

namespace neroshop {

class SamClient;
class Socks5Client;
class RoutingTable;
class KeyMapper;
struct BootstrapNode;

enum class NetworkType { I2P, Tor, Clearnet };

enum class NodeStatus { Dead, Inactive, Active };

struct Peer {
    std::string address;
    uint16_t port;
    std::string id;
    NodeStatus status;
    int distance;
};

class Node {
public:
    Node(bool local, const std::string& address = "", uint16_t port = 50881); // Binds a socket to a port and initializes the DHT
    ~Node();
    
    bool operator==(const Node& other) const {
        if (network_type_ != other.network_type_) {
            return false; // Different network types can't be equal
        }

        switch (network_type_) {
            case NetworkType::I2P:
                return i2p_address == other.i2p_address;
            case NetworkType::Tor:
                return onion_address == other.onion_address;
            default:
                throw std::runtime_error("Unsupported network type in equality comparison");
        }
    }

    void join(); // Sends a join message to the bootstrap peer to join the network
    void run(); // Main loop that listens for incoming messages
    void republish_once();
    bool validate(const std::string& key, const std::string& value); // Validates data before storing it
    int cache(const std::string& key, const std::string& value); // Cache data created by local client
    //---------------------------------------------------
    void persist_routing_table(const std::string& address, uint16_t port); // JIC bootstrap node faces outage and needs to recover
    void rebuild_routing_table(); // Re-builds routing table from data stored on disk

    void send_query(const std::string& destination, uint16_t port, const std::vector<uint8_t>& payload);
    
    bool send_ping(const std::string& destination, uint16_t port);
    std::vector<std::unique_ptr<Node>> send_find_node(const std::string& target_id, const std::string& destination, uint16_t port);
    int send_put(const std::string& key, const std::string& value);
    int send_store(const std::string& key, const std::string& value);
    std::string send_get(const std::string& key);
    std::string send_find_value(const std::string& key);
    void send_remove(const std::string& key);
    void send_map(const std::string& destination, uint16_t port); // Distributes indexing data to a single node
    void send_map_v2(const std::string& destination, uint16_t port);
    std::deque<Peer> send_get_providers(const std::string& data_hash);

    // DHT Query Types
    bool ping(const std::string& destination, uint16_t port); // A simple query to check if a node is online and responsive.
    std::vector<Node*> find_node(const std::string& target_id, int count) const;// override; // A query to find the contact information for a specific node in the DHT. // Finds the node closest to the target_id
    bool put(const std::string& key, const std::string& value); // A query to store a value in the DHT.    // Stores the key-value pair in the DHT
    bool store(const std::string& key, const std::string& value);
    std::string get(const std::string& key) const; // A query to get a specific value stored in the DHT.         // Retrieves the value associated with the key from the DHT
    std::string find_value(const std::string& key) const;
    bool remove(const std::string& key); // Remove a key-value pair from the DHT
    bool remove_all(); // Remove all data from in-memory hash table
    void map(const std::string& key, const std::string& value); // Maps search terms to keys
    void add_provider(const std::string& data_hash, const Peer& peer);
    void remove_providers(const std::string& data_hash);
    void remove_provider(const std::string& data_hash, const std::string& address, uint16_t port);
    std::deque<Peer> get_providers(const std::string& data_hash) const; // A query to get a list of peers for a specific torrent or infohash.
    
    // Setters
    void set_network_type(NetworkType network_type);
    
    // Getters
    std::string get_id() const; // get ID of this node
    SamClient * get_sam_client() const;
    std::string get_sam_version() const;
    //std::string get_public_key() const; // base64 public key
    //std::string get_private_key() const; // destination
    std::string get_address() const;
    uint16_t get_port() const;
    NetworkType get_network_type() const;
    std::string get_network_type_as_string() const;
    RoutingTable * get_routing_table() const;
    std::vector<Peer> get_peers() const; // Returns a list of connected peers
    int get_peer_count() const; // Returns the total number of nodes in routing table
    int get_active_peer_count() const;
    int get_idle_peer_count() const;
    NodeStatus get_status() const;
    std::string get_status_as_string() const;
    std::chrono::seconds get_uptime() const;
    std::string get_uptime_as_string() const;
    int get_distance(const std::string& node_id) const;
    std::vector<std::string> get_keys() const;
    std::vector<std::pair<std::string, std::string>> get_data() const;
    int get_data_count() const; // Returns the total number of in-memory hash table data
    int get_data_ram_usage() const;
    std::string get_cached(const std::string& key);
    KeyMapper * get_key_mapper() const;
    static const std::vector<BootstrapNode>& get_bootstrap_nodes(NetworkType type);
    
    void set_bootstrap(bool bootstrap);
    
    bool is_bootstrap_node() const;
    
    bool has_key(const std::string& key) const;
    bool is_hardcoded() const;
    static bool is_hardcoded(const std::string& address, uint16_t port, NetworkType network_type);
    
    bool has_key_cached(const std::string& key) const;
    bool has_value(const std::string& value) const;
    bool is_dead() const;
    static bool is_value_publishable(const std::string& value);
    
    bool is_running() const;
    // Friends
    friend class RoutingTable;
private:
    std::string get_i2p_address() const; // base32 public key
    std::string get_tor_address() const;
    std::string get_onion_address() const;
    ////std::string get_clearnet_address() const;
    // Callbacks
    void on_ping(const std::vector<uint8_t>& buffer, const std::string& destination);
    void on_map(const std::vector<uint8_t>& buffer, const std::string& destination);
    // Background Threads
    void heartbeat();
    void refresh(); // Periodically sends find_node queries
    void republish(); // Periodically republishes in-memory hash table data
    void purge(); // Periodically removes expired in-memory hash table data
    // Main loops
    void run_i2p();
    void run_tor();
    
    std::string generate_node_id(const std::string& address, int port = -1); // Use integer for checking invalids
    // Determines if node1 is closer to the target_id than node2
    bool is_closer(const std::string& target_id, const std::string& node1_id, const std::string& node2_id);
    //---------------------------------------------------
    bool set(const std::string& key, const std::string& value); // Updates the value without changing the key. set cannot be accessed directly but only through put
    bool verify(const std::string& value) const;
    void expire(const std::string& key, const std::string& value); // Removes any expired data from hash table
    bool validate_fields(const std::string& value);
    
    std::string id; // immutable and set only once in constructor so a mutex wouldn't make sense
    std::string i2p_address; // immutable and set only once in constructor so a mutex wouldn't make sense
    std::string onion_address;
    std::atomic<uint16_t> port_;
    NetworkType network_type_;
    std::unordered_map<std::string, std::string> data; // internal hash table that stores key-value pairs 
    mutable std::shared_mutex data_mutex;
    std::unordered_map<std::string, std::deque<Peer>> providers; // maps a data's hash (key) to a vector of Peers who have the data
    mutable std::shared_mutex providers_mutex;
    std::unique_ptr<SamClient> sam_client;
    std::unique_ptr<Socks5Client> socks5_client;
    std::unique_ptr<RoutingTable> routing_table;
    std::unique_ptr<KeyMapper> key_mapper;
    std::atomic<bool> bootstrap;
    std::atomic<int> check_counter; // Counter to track the number of consecutive failed checks
    std::unordered_map<std::string, std::promise<nlohmann::json>> pending_requests; // Shared between sender and listener
    std::mutex pending_mutex;
    // Connection pool
    std::unordered_map<std::string, std::unique_ptr<Socks5Client>> tor_peers;
    std::mutex tor_peers_mutex;
    void handle_tor_message(std::vector<uint8_t> message, const std::string& sender_onion, uint16_t sender_port);
    std::chrono::steady_clock::time_point start_time;
    // For all background threads
    void stop_threads();
    static void signal_handler(int signum);
    static Node* instance;
    std::atomic<bool> running;
    std::mutex cv_mutex;
    std::condition_variable cv;
};

}
