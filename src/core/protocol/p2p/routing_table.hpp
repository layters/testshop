#pragma once

#include <iostream>
#include <memory> // std::unique_ptr
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>

#include "../../../neroshop_config.hpp"

namespace neroshop {

class Node; // forward declaration

class RoutingTable {
private:     
    friend class Node;
    std::string my_node_id;
    std::vector<Node *> nodes;  // List of nodes in the DHT
    mutable std::unordered_map<int, std::vector<std::unique_ptr<Node>>> buckets;  // Routing table buckets
    // Declare a mutex to protect access to the routing table
    std::shared_mutex routing_table_mutex; // Shared mutex for routing table access
public:
    // Initialize the routing table with a list of nodes
    RoutingTable(const std::vector<Node *>& nodes);

    // Add a new node to the routing table
    bool add_node(std::unique_ptr<Node> node);//void add_node(const Node& node);
    
    bool remove_node(const std::string& node_ip, uint16_t node_port);
    bool remove_node(const std::string& node_id);

    // Find the bucket that a given node belongs in
    int find_bucket(const std::string& node_id) const;
    
    std::optional<std::reference_wrapper<neroshop::Node>> get_node(const std::string& node_id);// const;

    std::vector<Node*> find_closest_nodes(const std::string& key, int count = NEROSHOP_DHT_MAX_CLOSEST_NODES);// const;// K or count is the maximum number of closest nodes to return
    Node* find_node_by_id(const std::string& node_id) const;

    bool split_bucket(int bucket_index);

    // Print the contents of the routing table
    void print_table() const;
    
    int get_bucket_count() const;
    int get_node_count() const;
    int get_node_count(int bucket_index) const;
    
    bool is_bucket_full(int bucket_index) const;
    bool are_buckets_full() const;
    
    bool has_node(const std::string& ip_address, uint16_t port);
    bool has_node(const std::string& node_id);// const;
    
    static std::string calculate_distance(const std::string& hash1, const std::string& hash2);  
};

}
