#include "hash_ring.hpp"

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <functional>

#include "node.hpp"

//neroshop::Ring::Ring() {}

//-----------------------------------------------------------------------------

// Hash function that returns a hash value for a given string
std::size_t neroshop::Ring::hash(const std::string& key) {
    // TODO: Implement a suitable hash function
    return std::hash<std::string>{}(key);
}

//-----------------------------------------------------------------------------

void neroshop::Ring::add_node(const std::shared_ptr<Node>& node) {
    nodes_.push_back(node);
    std::sort(nodes_.begin(), nodes_.end(), [](const std::shared_ptr<Node>& n1, const std::shared_ptr<Node>& n2) {
        return n1->get_id() < n2->get_id();
    });
}

void neroshop::Ring::remove_node(const std::shared_ptr<Node>& node) {
    auto it = std::find(nodes_.begin(), nodes_.end(), node);
    if (it != nodes_.end()) {
        nodes_.erase(it);
    }
}

//-----------------------------------------------------------------------------

std::shared_ptr<neroshop::Node> neroshop::Ring::get_node_for_key(const std::string& key) const {
    std::size_t hash_value = hash(key);
    auto it = std::upper_bound(nodes_.begin(), nodes_.end(), hash_value, [](std::size_t hash_value, const std::shared_ptr<Node>& node) {
        return hash_value < hash(node->get_id());
    });
    if (it == nodes_.end()) {
        return nodes_.front();
    } else {
        return *it;
    }
}

std::vector<std::shared_ptr<neroshop::Node>> neroshop::Ring::get_nodes_for_key(const std::string& key, int replication_factor) const {
    std::size_t hash_value = hash(key);
    std::vector<std::shared_ptr<Node>> nodes;
    for (int i = 0; i < replication_factor; ++i) {
        auto it = std::upper_bound(nodes_.begin(), nodes_.end(), hash_value, [](std::size_t hash_value, const std::shared_ptr<Node>& node) {
            return hash_value < hash(node->get_id());
        });
        if (it == nodes_.end()) {
            nodes.push_back(nodes_.front());
        } else {
            nodes.push_back(*it);
        }
        hash_value = hash(std::to_string(hash_value) + std::to_string(i));
    }
    return nodes;
}

//-----------------------------------------------------------------------------    
