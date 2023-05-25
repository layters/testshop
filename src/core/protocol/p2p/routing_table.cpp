#include "routing_table.hpp"

#include <bitset>
#include <functional> // std::hash
#include <regex>
#include <cassert>

#include <openssl/bn.h> // BIGNUM

#include "node.hpp"

// Initialize the routing table with a list of nodes
neroshop::RoutingTable::RoutingTable(const std::vector<Node *>& nodes) : nodes(nodes) {
    // A single bucket can store up to NEROSHOP_DHT_MAX_BUCKET_SIZE nodes
    for (int i = 0; i < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS; ++i) {
        buckets.emplace(i, std::vector<std::unique_ptr<Node>>{});
    }//std::cout << "Buckets size: " << buckets.size() << "\n";
    // Add nodes to the routing table
    for (const auto& node_ptr : nodes) {
        if (node_ptr == nullptr) {
            continue;
        }
        const std::string& node_id = node_ptr->get_id();
        const std::string& key_hash = calculate_distance(node_id, my_node_id); // Calculate distance from the current node
        int bucket_index = 0;
        while (bucket_index < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS && (key_hash[bucket_index] == 0)) {
            bucket_index++;
        }
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
    if (bucket_index < 0 || bucket_index >= NEROSHOP_DHT_ROUTING_TABLE_BUCKETS) {
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

    // Acquire a lock to ensure exclusive access to the routing table
    std::unique_lock<std::shared_mutex> write_lock(routing_table_mutex);
    
    // Add the node to the bucket
    bucket.push_back(std::move(node));
    std::cout << "\033[0;36m" << (bucket.back().get()->get_ip_address() + ":" + std::to_string(bucket.back().get()->get_port())) << "\033[0m added to routing table\n";

    // Check if bucket splitting is required and perform the split
    if (bucket.size() > NEROSHOP_DHT_MAX_BUCKET_SIZE) { // buckets have a max size of NEROSHOP_DHT_MAX_BUCKET_SIZE nodes
        bool split_success = split_bucket(bucket_index);
        if (!split_success) {
            std::cerr << "Error: unable to split bucket " << bucket_index << ".\n";
            return false;
        }
        std::cout << "Bucket " << bucket_index << " split into " << bucket_index << " and " << (bucket_index + 1) << ".\n";
    }    
    
    return true;
}

bool neroshop::RoutingTable::remove_node(const std::string& node_ip, uint16_t node_port) {
    assert(node_ip != "127.0.0.1" && "Routing table only stores public IP addresses");
    for (auto& bucket : buckets) {
        std::vector<std::unique_ptr<Node>>& nodes = bucket.second;
        std::unique_lock<std::shared_mutex> write_lock(routing_table_mutex);  // Acquire an exclusive lock
        for (auto it = nodes.begin(); it != nodes.end(); /* no increment here */) {
            if ((*it)->get_ip_address() == node_ip && (*it)->get_port() == node_port) {
                std::cout << "\033[0;91m" << node_ip << ":" << node_port << "\033[0m removed from routing table\n";
                it = nodes.erase(it);  // erase returns the iterator to the next valid element
                return true;
            } else {
                ++it;  // increment the iterator only if element not found
            }
        }
    }

    std::cerr << "Error: node with IP " << node_ip << " and port " << node_port << " not found in the routing table.\n";
    return false;
}

bool neroshop::RoutingTable::remove_node(const std::string& node_id) {
    for (auto& bucket : buckets) {
        std::vector<std::unique_ptr<Node>>& nodes = bucket.second;
        std::unique_lock<std::shared_mutex> write_lock(routing_table_mutex);  // Acquire an exclusive lock
        for (auto it = nodes.begin(); it != nodes.end(); /* no increment here */) {
            if ((*it)->get_id() == node_id) {
                std::cout << "\033[0;91m" << node_id << "\033[0m removed from routing table\n";
                it = nodes.erase(it);  // erase returns the iterator to the next valid element
                return true;
            } else {
                ++it;  // increment the iterator only if element not found
            }
        }
    }

    std::cerr << "Error: node with ID " << node_id << " not found in the routing table.\n";
    return false;
}

// Find the index of the bucket that a given node identifier belongs to
int neroshop::RoutingTable::find_bucket(const std::string& node_id) const {//(int node_id) const {
    std::string distance = calculate_distance(node_id, my_node_id);

    // Find the index of the non-zero bit from the left in the distance string
    int bucket_index = NEROSHOP_DHT_ROUTING_TABLE_BUCKETS - 1;
    while (bucket_index >= 0 && distance[bucket_index] == '0') {
        bucket_index--;
    }

    // Return the index of the bucket
    return bucket_index;
}

bool neroshop::RoutingTable::split_bucket(int bucket_index) {
    // Check if splitting is possible (requires at least one node in the bucket)
    if (buckets[bucket_index].empty()) {
        return false;
    }

    // Create two new empty buckets
    std::vector<std::unique_ptr<Node>> new_bucket1;
    std::vector<std::unique_ptr<Node>> new_bucket2;

    // Move nodes from the original bucket to the new buckets
    for (auto& node : buckets[bucket_index]) {
        // Calculate the new bucket index for the node
        unsigned long long new_bucket_index = find_bucket(node->get_id());
        if (new_bucket_index == bucket_index) {
            new_bucket1.push_back(std::move(node));
    } else {
          new_bucket2.push_back(std::move(node));
        }
    }

    // Update the routing table with the new bucket configuration
    buckets[bucket_index] = std::move(new_bucket1);
    buckets[bucket_index + 1] = std::move(new_bucket2);

    return true;
}


std::optional<std::reference_wrapper<neroshop::Node>> neroshop::RoutingTable::get_node(const std::string& node_id) {//const {
    const std::string& key_hash = calculate_distance(node_id, my_node_id); // Calculate distance from the current node
    int bucket_index = 0;
    while (bucket_index < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS && (key_hash[bucket_index] == 0)) {
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
    std::string key_hash = key;//hash_to_string(key);
    int bucket_index = 0;
    while (bucket_index < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS && (key_hash[bucket_index] == 0)) {
        bucket_index++;
    }

    if (bucket_index >= NEROSHOP_DHT_ROUTING_TABLE_BUCKETS) {
        return closest_nodes; // routing table is empty
    }

    // Check the bucket containing the key
    const auto& bucket = buckets[bucket_index];
    if (!bucket.empty()) {
        // Find the closest node(s) in the bucket
        std::map<std::string, neroshop::Node*> closest_nodes_map;
        for (const auto& node : bucket) {
            std::string node_hash = node->get_id();
            std::string distance_to_node = calculate_distance(node_hash, key_hash);
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
    while (closest_nodes.size() < count && (left_bucket_index >= 0 || right_bucket_index < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS)) {
        if (left_bucket_index >= 0) {
            const auto& left_bucket = buckets[left_bucket_index];
            if (!left_bucket.empty()) {
                std::map<std::string, neroshop::Node*> closest_nodes_map;
                for (const auto& node : left_bucket) {
                    std::string node_hash = node->get_id();
                    std::string distance_to_node = calculate_distance(node_hash, key_hash);
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
        if (right_bucket_index < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS) {
            const auto& right_bucket = buckets[right_bucket_index];
            if (!right_bucket.empty()) {
                std::map<std::string, neroshop::Node*> closest_nodes_map;
                for (const auto& node : right_bucket) {
                    std::string node_hash = node->get_id();
                    std::string distance_to_node = calculate_distance(node_hash, key_hash);
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

//-----------------------------------------------------------------------------

int neroshop::RoutingTable::get_bucket_count() const {
    return buckets.size();
}

int neroshop::RoutingTable::get_node_count() const {
    int count = 0;
    for (const auto& [bucket_index, bucket_nodes] : buckets) {
        count += bucket_nodes.size();
    }
    return count;
}

int neroshop::RoutingTable::get_node_count(int bucket_index) const {
    assert(bucket_index >= 0 && bucket_index < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS);
    return buckets[bucket_index].size();
}

//-----------------------------------------------------------------------------

bool neroshop::RoutingTable::is_bucket_full(int bucket_index) const {
    if (buckets.find(bucket_index) == buckets.end()) {
        // Bucket does not exist, it is considered empty
        return false;
    }

    const std::vector<std::unique_ptr<Node>>& bucket = buckets[bucket_index];
    return bucket.size() >= NEROSHOP_DHT_MAX_BUCKET_SIZE;
}

bool neroshop::RoutingTable::are_buckets_full() const {
    for (int i = 0; i < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS; i++) {
        if (!is_bucket_full(i)) {
            return false;
        }
    }
    return true;
}

bool neroshop::RoutingTable::has_node(const std::string& ip_address, uint16_t port) {
    assert(ip_address != "127.0.0.1" && "Routing table only stores public IP addresses");
    // Iterate over the buckets
    for (const auto& bucket : buckets) {
        // Iterate over the nodes in the bucket
        for (const auto& node : bucket.second) {
            // Check if the node's IP address and port match
            if (node->get_ip_address() == ip_address && node->get_port() == port) {
                return true;
            }
        }
    }
    // Node not found in the routing table
    return false;
}

// CAUTION: node_ids may change so it's recommended to use the alternative has_node() function
bool neroshop::RoutingTable::has_node(const std::string& node_id) {//const {
    const std::string& key_hash = calculate_distance(node_id, my_node_id); // Calculate distance from the current node
    int bucket_index = 0;
    while (bucket_index < NEROSHOP_DHT_ROUTING_TABLE_BUCKETS && (key_hash[bucket_index] == 0)) {
        bucket_index++;
    }

    if (bucket_index >= NEROSHOP_DHT_ROUTING_TABLE_BUCKETS) {
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

//-----------------------------------------------------------------------------

// Print the contents of the routing table
void neroshop::RoutingTable::print_table() const {
    for (const auto& [bucket_index, bucket_nodes] : buckets) {
        if (!bucket_nodes.empty()) { // Check if bucket is not empty
            std::cout << "Bucket " << bucket_index << ": ";
            for (auto& node : bucket_nodes) {
                std::cout << node->get_ip_address() << ":" << node->get_port() << " ";
                //std::cout << "[" << calculate_distance(node->get_id(), my_node_id) << "] ";
            }
            std::cout << std::endl;
        }
    }
}

//-----------------------------------------------------------------------------

// Calculate the distance between two hash values
std::string neroshop::RoutingTable::calculate_distance(const std::string& hash1, const std::string& hash2) {
    std::string distance;
    const size_t numBits = hash1.size() * 8;
    for (size_t i = 0; i < numBits; i++) {
        // Get the byte and bit position of the current bit
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;

        // Extract the corresponding bits from the two hash values
        char bit1 = (hash1[byteIndex] >> bitIndex) & 1;
        char bit2 = (hash2[byteIndex] >> bitIndex) & 1;

        // Perform bitwise XOR on the bits
        char xorValue = bit1 ^ bit2;

        // Convert XOR value to binary string representation and append it to the distance string
        std::string xorStr = std::bitset<8>(xorValue).to_string();
        distance += xorStr;
    }
    
    return distance;
}



// Determine the bucket index for a hash value
/*unsigned int getBucketIndex(const std::string& hash) {
    // Extract the most significant bits (e.g., first 8 bits)
    char msb = hash[0];
    unsigned int bucketIndex = static_cast<unsigned int>(msb) % NEROSHOP_DHT_ROUTING_TABLE_BUCKETS;
    return bucketIndex;
}*/
