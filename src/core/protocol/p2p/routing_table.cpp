#include "routing_table.hpp"

#include <functional> // std::hash

#include "node.hpp"

    // Initialize the routing table with a list of nodes
    neroshop::RoutingTable::RoutingTable(const std::vector<Node *>& nodes) : nodes(nodes) {
        // A routing table consists of 160 buckets which can hold up to 20 nodes
        /*for (int i = 0; i < 160; i++) {
            buckets[i] = {};
        }*/ // This will ensure that buckets is properly initialized with empty vectors for each bucket index.
    }

    // Add a new node to the routing table
    bool neroshop::RoutingTable::add_node(Node* node) {//(const Node& node) {
        if (!node) {
            std::cerr << "Error: cannot add a null node to the routing table.\n";
            return false;
        }
        
        std::string node_id = node->get_id();
        // Find the bucket that the node belongs in
        int bucket_index = find_bucket(node_id);
        
        // Add the node to the appropriate bucket
        if (bucket_index < 0 || bucket_index >= 160) {
            std::cerr << "Error: invalid bucket index " << bucket_index << " for node with ID " << node_id << ".\n";
            return false;
        }

        buckets[bucket_index].push_back(std::move(node));
        std::cout << "Node \033[0;36m" << (node->get_ip_address() + ":" + std::to_string(node->get_port())) << "\033[0m added to routing table\n";
        return true;
    }

    // Find the bucket that a given node belongs in
    int neroshop::RoutingTable::find_bucket(const std::string& node_id) const {//(int node_id) const {
        unsigned int id_num = hash_to_int(node_id);
        int bucket_index = 0;
        while (bucket_index < 160 && (id_num & (1 << bucket_index)) == 0) {
            bucket_index++;
        }
        return bucket_index;
    }

    std::optional<neroshop::Node*> neroshop::RoutingTable::find_node(const std::string& node_id) {//const {
        unsigned int id_num = hash_to_int(node_id);
        int bucket_index = 0;
        while (bucket_index < 160 && (id_num & (1 << bucket_index)) == 0) {
            bucket_index++;
        }
        
        if (bucket_index >= buckets.size()) {
            return std::nullopt; // bucket is empty
        }
        const auto& bucket = buckets[bucket_index];
        for (const auto& node : bucket) {
            if (node->get_id() == node_id) {
                return node;
            }
        }
        return std::nullopt; // node not found in bucket
    }

    // Print the contents of the routing table
    void neroshop::RoutingTable::print_table() const {
        for (const auto& [bucket_index, bucket_nodes] : buckets) {
            std::cout << "Bucket " << bucket_index << ": ";
            for (auto& node : bucket_nodes) {
                std::cout << node->get_ip_address() << ":" << node->get_port() << " ";
            }
            std::cout << std::endl;
        }
    }


unsigned int neroshop::RoutingTable::hash_to_int(const std::string& hash) {
    std::hash<std::string_view> hash_fn;
    return hash_fn(hash);
}    
