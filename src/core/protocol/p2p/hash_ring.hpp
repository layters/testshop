#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <memory> // smart pointers
#include <unordered_map>

#define DEFAULT_REPLICATION_FACTOR 3

// Consistent hashing rings use a technique called the hash function to determine which node(s) should be responsible for a key. The hash function takes the key as input and produces a numerical value, typically a 32-bit or 64-bit integer. The hash value is then mapped onto a ring-like structure, often represented as a circle.
// Each node in the consistent hashing ring is assigned a position on the ring based on its own identifier or hash value. When a key needs to be mapped to a node, the hash value of the key is calculated. Starting from that point on the ring, the algorithm moves clockwise until it encounters the first node in that direction. That node is responsible for storing the key.
namespace neroshop {

class Node; // forward declaration

// A consistent hashing ring
class Ring {
public:
    Ring() = default;
    
    void add_node(const std::shared_ptr<Node>& node); // Add a node to the ring
    void remove_node(const std::shared_ptr<Node>& node); // Remove a node from the ring
    
    std::shared_ptr<neroshop::Node> get_node_for_key(const std::string& key) const; // Get the node that is responsible for a given key
    std::vector<std::shared_ptr<Node>> get_nodes_for_key(const std::string& key, int replication_factor) const; // Get the nodes that are responsible for a given key
private:
    std::vector<std::shared_ptr<Node>> nodes_;
    static std::size_t hash(const std::string& key); // Hash function that returns a hash value for a given string
};

}
