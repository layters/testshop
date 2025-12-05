#pragma once

#include <iostream>
#include <string>
#include <array>          // for std::array
#include <vector>         // for std::vector
#include <memory>         // for std::shared_ptr, std::unique_ptr
#include <shared_mutex>   // for std::shared_mutex

#include "../../../neroshop_config.hpp"

namespace neroshop {

class Node; // forward declaration

constexpr int BUCKET_COUNT = NEROSHOP_DHT_ROUTING_TABLE_BUCKETS; // should be 256
using xor_id = std::array<uint8_t, BUCKET_COUNT / 8>; // should be equivalent to: std::array<uint8_t, 32>

struct Bucket {
    std::vector<std::shared_ptr<Node>> nodes;
    mutable std::shared_mutex mutex;
    std::chrono::steady_clock::time_point last_changed;

    Bucket() : last_changed(std::chrono::steady_clock::time_point::min()) {}
};

class RoutingTable {
public:
    // Initialize the routing table with a node ID
    explicit RoutingTable(const std::string& node_id); // hexadecimal string (SHA-3-256) is converted to byte array (XOR ID)
    explicit RoutingTable(const xor_id&      node_id); // XOR ID

    // Add a new node to the routing table
    bool add_node(std::unique_ptr<Node> node);
    
    bool remove_node(const std::string& address, uint16_t port);
    bool remove_node(const std::string& node_id);

    //bool split_bucket(int bucket_index);

    // Print the contents of the routing table
    void print_table() const;    
    
    // Get a random ID within a bucket's range
    xor_id generate_random_node_id(int bucket_index) const;
    std::string generate_random_hex_string(int bucket_index) const;
    
    std::vector<std::weak_ptr<Node>> find_closest_nodes(const std::string& key, int count = NEROSHOP_DHT_MAX_CLOSEST_NODES);// const;// K or count is the maximum number of closest nodes to return
    std::weak_ptr<Node> get_node(const std::string& node_id) const;
    std::vector<std::weak_ptr<Node>> get_nodes() const;
    
    // Find the bucket index that a given node belongs in
    int get_bucket_index(const std::string& node_id) const;
    static int get_bucket_index(const std::string& lhs, const std::string& rhs);
    
    int get_bucket_count() const;
    int get_node_count() const;
    int get_node_count(int bucket_index) const;
    
    bool is_bucket_full(int bucket_index) const;
    bool are_buckets_full() const;
    
    bool has_node(const std::string& ip_address, uint16_t port);
    bool has_node(const std::string& node_id);// const;
    
    friend class Node;
private:
    xor_id id; // Local node's byte array ID
    std::array<Bucket, BUCKET_COUNT> buckets;
};

}
