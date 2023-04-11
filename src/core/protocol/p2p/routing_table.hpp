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
    std::unordered_map<int, std::vector<Node *>> buckets;  // Routing table buckets
public:
    // Initialize the routing table with a list of nodes
    RoutingTable(const std::vector<Node *>& nodes);

    // Add a new node to the routing table
    bool add_node(Node * node);//void add_node(const Node& node);

    // Find the bucket that a given node belongs in
    int find_bucket(const std::string&/*int*/ node_id) const;
    
    std::optional<Node*> find_node(const std::string& node_id);// const;

    // Print the contents of the routing table
    void print_table() const;

};

}
