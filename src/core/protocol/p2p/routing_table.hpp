#pragma once

#include <iostream>
#include <memory> // std::unique_ptr
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace neroshop {

class Node; // forward declaration

class RoutingTable {
private:     
    // Convert hash to string
    static unsigned int hash_to_int(const std::string& hash);
    std::vector<Node *> nodes;  // List of nodes in the DHT
    std::unordered_map<int, std::vector<std::unique_ptr<Node>>> buckets;  // Routing table buckets
public:
    // Initialize the routing table with a list of nodes
    RoutingTable(const std::vector<Node *>& nodes);

    // Add a new node to the routing table
    bool add_node(std::unique_ptr<Node> node);//void add_node(const Node& node);
    
    bool remove_node(const std::string& node_id);

    // Find the bucket that a given node belongs in
    int find_bucket(const std::string& node_id) const;
    
    std::optional<std::reference_wrapper<neroshop::Node>> get_node(const std::string& node_id);// const;

    std::vector<Node*> find_closest_nodes(const std::string& key, int count = 10);// const;// K or count is the maximum number of closest nodes to return

    // Print the contents of the routing table
    void print_table() const;

    bool has_node(const std::string& node_id);// const;
};

}
