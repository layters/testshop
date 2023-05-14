#include "routing_table.hpp"

#include <bitset>
#include <functional> // std::hash
#include <regex>

#include "node.hpp"

const int NUM_BUCKETS = 256; // recommended to use a number of buckets that is equal to the number of bits in the node id (sha-3-256 so 256 bits)

// Initialize the routing table with a list of nodes
neroshop::RoutingTable::RoutingTable(const std::vector<Node *>& nodes) : nodes(nodes) {
    // A single bucket can store up to 20 nodes
    for (int i = 0; i < NUM_BUCKETS; ++i) {
        buckets.emplace(i, std::vector<std::unique_ptr<Node>>{});
    }//std::cout << "Buckets size: " << buckets.size() << "\n";
    // Add nodes to the routing table
    for (const auto& node_ptr : nodes) {
        if (node_ptr == nullptr) {
            continue;
        }
        const int bucket_index = hash_to_int(node_ptr->get_id());
        std::cout << "Node stored in bucket " << bucket_index << "\n";
        std::unique_ptr<Node> node_uptr(node_ptr);
        buckets[bucket_index].push_back(std::move(node_uptr));
    }
}

// Add a new node to the routing table
bool neroshop::RoutingTable::add_node(std::unique_ptr<Node> node) {//(const Node& node) {
    if (!node.get()) {
        std::cerr << "Error: cannot add a null node to the routing table.\n";
        return false;
    }
        
    std::string node_id = node->get_id();
    // Find the bucket that the node belongs in
    int bucket_index = find_bucket(node_id);
        
    // Add the node to the appropriate bucket
    if (bucket_index < 0 || bucket_index >= NUM_BUCKETS) {
        std::cerr << "Error: invalid bucket index " << bucket_index << " for node with ID " << node_id << ".\n";
        return false;
    }

    // Check if the node already exists in the bucket
    auto& bucket = buckets.at(bucket_index);
    for (const auto& n : bucket) {
        if (n->get_id() == node_id) {
            std::cout << "\033[0;33m" << (node->get_ip_address() + ":" + std::to_string(node->get_port())) << "\033[0m already exists in routing table\n";
            return true;
        }
    }

    // Add the node to the bucket
    bucket.push_back(std::move(node));
    std::cout << "\033[0;36m" << (bucket.back().get()->get_ip_address() + ":" + std::to_string(bucket.back().get()->get_port())) << "\033[0m added to routing table\n";
    return true;
}

bool neroshop::RoutingTable::remove_node(const std::string& node_id) {
    int bucket_index = find_bucket(node_id);

    if (bucket_index < 0 || bucket_index >= NUM_BUCKETS) {
        std::cerr << "Error: invalid bucket index " << bucket_index << " for node with ID " << node_id << ".\n";
        return false;
    }

    std::vector<std::unique_ptr<Node>>& bucket = buckets[bucket_index];
    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
        if ((*it)->get_id() == node_id) {
            std::cout << "\033[0;91m" << (*it)->get_ip_address() << ":" << (*it)->get_port() << "\033[0m removed from routing table\n";
            bucket.erase(it);
            return true;
        }
    }

    std::cerr << "Error: node with ID " << node_id << " not found in bucket " << bucket_index << ".\n";
    return false;
}

// Find the index of the bucket that a given node identifier belongs to
int neroshop::RoutingTable::find_bucket(const std::string& node_id) const {//(int node_id) const {
    unsigned int id_num = hash_to_int(node_id);
    int bucket_index = 0;
    while (bucket_index < NUM_BUCKETS && (id_num & (1 << bucket_index)) == 0) {
        bucket_index++;
    }
    return bucket_index;
}

std::optional<std::reference_wrapper<neroshop::Node>> neroshop::RoutingTable::get_node(const std::string& node_id) {//const {
    unsigned int id_num = hash_to_int(node_id);
    int bucket_index = 0;
    while (bucket_index < NUM_BUCKETS && (id_num & (1 << bucket_index)) == 0) {
        bucket_index++;
    }
        
    if (bucket_index >= buckets.size()) {
        return std::nullopt; // bucket is empty
    }
    const auto& bucket = buckets[bucket_index];
    for (const auto& node : bucket) {
        if (node->get_id() == node_id) {
            return std::ref(*node);
        }
    }
    return std::nullopt; // node not found in bucket
}

// un-tested
std::vector<neroshop::Node*> neroshop::RoutingTable::find_closest_nodes(const std::string& key, int count) {
    std::vector<neroshop::Node*> closest_nodes;
    unsigned int key_hash = hash_to_int(key);
    int bucket_index = 0;
    while (bucket_index < NUM_BUCKETS && (key_hash & (1 << bucket_index)) == 0) {
        bucket_index++;
    }

    if (bucket_index >= NUM_BUCKETS) {
        return closest_nodes; // routing table is empty
    }

    // Check the bucket containing the key
    const auto& bucket = buckets[bucket_index];
    if (!bucket.empty()) {
        // Find the closest node(s) in the bucket
        std::map<unsigned int, neroshop::Node*> closest_nodes_map;
        for (const auto& node : bucket) {
            unsigned int node_hash = hash_to_int(node->get_id());
            unsigned int distance_to_node = node_hash ^ key_hash;
            closest_nodes_map[distance_to_node] = node.get();
        }
        for (const auto& closest_node_pair : closest_nodes_map) {
            closest_nodes.push_back(closest_node_pair.second);
            if (closest_nodes.size() == count) {
                return closest_nodes;
            }
        }
    }

    // Check the adjacent buckets for the closest nodes
    int left_bucket_index = bucket_index - 1;
    int right_bucket_index = bucket_index + 1;
    while (closest_nodes.size() < count && (left_bucket_index >= 0 || right_bucket_index < NUM_BUCKETS)) {
        if (left_bucket_index >= 0) {
            const auto& left_bucket = buckets[left_bucket_index];
            if (!left_bucket.empty()) {
                std::map<unsigned int, neroshop::Node*> closest_nodes_map;
                for (const auto& node : left_bucket) {
                    unsigned int node_hash = hash_to_int(node->get_id());
                    unsigned int distance_to_node = node_hash ^ key_hash;
                    closest_nodes_map[distance_to_node] = node.get();
                }
                for (const auto& closest_node_pair : closest_nodes_map) {
                    closest_nodes.push_back(closest_node_pair.second);
                    if (closest_nodes.size() == count) {
                        return closest_nodes;
                    }
                }
            }
            left_bucket_index--;
        }
        if (right_bucket_index < NUM_BUCKETS) {
            const auto& right_bucket = buckets[right_bucket_index];
            if (!right_bucket.empty()) {
                std::map<unsigned int, neroshop::Node*> closest_nodes_map;
                for (const auto& node : right_bucket) {
                    unsigned int node_hash = hash_to_int(node->get_id());
                    unsigned int distance_to_node = node_hash ^ key_hash;
                    closest_nodes_map[distance_to_node] = node.get();
                }
                for (const auto& closest_node_pair : closest_nodes_map) {
                    closest_nodes.push_back(closest_node_pair.second);
                    if (closest_nodes.size() == count) {
                        return closest_nodes;
                    }
                }
            }
            right_bucket_index++;
        }
    }

    return closest_nodes;
}
    

bool neroshop::RoutingTable::has_node(const std::string& node_id) {//const {
    unsigned int node_hash = hash_to_int(node_id);
    int bucket_index = 0;
    while (bucket_index < NUM_BUCKETS && (node_hash & (1 << bucket_index)) == 0) {
        bucket_index++;
    }

    if (bucket_index >= NUM_BUCKETS) {
        return false; // routing table is empty
    }

    const auto& bucket = buckets[bucket_index];
    for (const auto& node : bucket) {
        if (node->get_id() == node_id) {
            return true;
        }
    }

    return false;
}

// Print the contents of the routing table
void neroshop::RoutingTable::print_table() const {
    for (const auto& [bucket_index, bucket_nodes] : buckets) {
        if (!bucket_nodes.empty()) { // Check if bucket is not empty
            std::cout << "Bucket " << bucket_index << ": ";
            for (auto& node : bucket_nodes) {
                std::cout << node->get_ip_address() << ":" << node->get_port() << " ";
            }
            std::cout << std::endl;
        }
    }
}

unsigned int neroshop::RoutingTable::hash_to_int(const std::string& hash) {
    if (hash.size() != 64) {
        throw std::invalid_argument("Hash length must be 64 characters (SHA-3-256)");
    }
    // Validate the hash using a regular expression
    static const std::regex hex_regex("^[0-9a-fA-F]{64}$");
    if (!std::regex_match(hash, hex_regex)) {
        throw std::invalid_argument("Hash must be a 64-character hexadecimal string");
    }
    // Convert the entire hash to an integer
    unsigned int result = 0;
    std::stringstream ss;
    ss << std::hex << hash;
    ss >> result;    
    return result % NUM_BUCKETS; // limit result to the range [0, 256)
}
