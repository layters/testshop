#include "routing_table.hpp"

#include "node.hpp"
#include "../../tools/logger.hpp"

#include <cassert>

namespace neroshop {

//-----------------------------------------------------------------------------

uint8_t hex_char_to_byte(char c) {
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return 10 + (c - 'a');
    if ('A' <= c && c <= 'F') return 10 + (c - 'A');
    throw std::invalid_argument("Invalid hex character");
}

//-----------------------------------------------------------------------------

xor_id hex_string_to_node_id(const std::string& hex) {
    if (hex.size() != 64) throw std::invalid_argument("SHA3-256 hex string must be 64 characters");

    xor_id node_id;
    for (size_t i = 0; i < 32; ++i) {
        try {
            node_id[i] = (hex_char_to_byte(hex[2 * i]) << 4) | hex_char_to_byte(hex[2 * i + 1]);
        } catch (const std::invalid_argument& e) {
            log_error("hex_string_to_node_id: Error converting hex string: {}", e.what());
        }
    }
    return node_id;
}

//-----------------------------------------------------------------------------

// Portable MSB Finder for uint8_t (returns the position of the most significant set bit (from left, 0-indexed) in an 8-bit value)
inline int msb_index_from_left(uint8_t byte) {
    if (byte == 0) return -1; // No set bit

    for (int i = 0; i < 8; ++i) {
        if (byte & (1 << (7 - i))) {
            return i;
        }
    }

    return -1; // Should never happen
}

//-----------------------------------------------------------------------------

inline uint32_t xor_distance_bit_index(const xor_id& a, const xor_id& b) {
    for (size_t i = 0; i < a.size(); ++i) {
        uint8_t xor_byte = a[i] ^ b[i];
        if (xor_byte != 0) {
            #ifdef NEROSHOP_DEBUG0
            std::cout << "Differing byte at index i: " << i << std::endl;
            std::cout << "XOR byte: " << static_cast<int>(xor_byte) << " (Binary: ";
            for (int bit = 7; bit >= 0; --bit) {
                std::cout << ((xor_byte >> bit) & 0x1);
            }
            std::cout << ")" << std::endl;
            #endif

            // The index of the MSB from the left (0-indexed) within the 8-bit byte is simply the number of leading zeros within the 8 bits.
            int j = msb_index_from_left(xor_byte);

            #ifdef NEROSHOP_DEBUG0
            std::cout << "  Using msb_index_from_left:" << std::endl;
            std::cout << "    Leading zeros in 8-bit byte: " << j << std::endl;
            std::cout << "    MSB set bit found at bit index j (from left): " << j << std::endl;
            #endif

            // Calculate the distance based on the position from the MSB of the entire ID.
            // The index j from msb_index_from_left is the number of leading zeros in the 8-bit value,
            // which is also the index of the most significant set bit from the left (0-indexed).
            uint32_t distance = (i * 8) + j;
            #ifdef NEROSHOP_DEBUG0
            std::cout << "  Calculated distance: (i * 8) + j = (" << i << " * 8) + " << j << " = " << distance << std::endl;
            #endif
            return distance;
        }
    }
    
    uint32_t distance = a.size() * 8;
    #ifdef NEROSHOP_DEBUG
    if(distance == 256) {
        log_debug("xor_distance_bit_index: Identical IDs (node queried itself)");
    }
    #endif
    
    return distance; // 256 = matching node IDs which is invalid. XOR operation can only produce distances in the range 0 to 255
}

//-----------------------------------------------------------------------------

void print_xor_id(const std::string& node_id_hex) {
    // Print the node XOR ID in hexadecimal format
    // Each pair of hexadecimal digits (21, 5c, 27, etc.) corresponds to a byte of the hash.
    // Each of the 32 bytes can be accessed using: node_id[i]
    xor_id node_id = hex_string_to_node_id(node_id_hex);
    for (size_t i = 0; i < node_id.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(node_id[i]) << " ";
    }
    std::cout << std::endl;
}

void print_xor_id(const xor_id& node_id) {
    for (size_t i = 0; i < node_id.size(); ++i) {
      std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(node_id[i]) << " ";
    }
    std::cout << std::endl;
}

//-----------------------------------------------------------------------------

RoutingTable::RoutingTable(const std::string& node_id_hex) : id(hex_string_to_node_id(node_id_hex)) { // node_id_hex is a SHA-3-256 hexidecimal string
    // Buckets and mutexes are statically sized (std::array), no dynamic allocation needed.
    
    ////print_xor_id(this->id);
    
    // There are 32 bytes (64 hexadecimal digits) in a SHA-3-256 hash
    int bytes = node_id_hex.length() / 2;
    assert(bytes == 32 && "Invalid number of bytes in 64-length hexadecimal string (SHA3-256)");
    assert(this->id.size() == bytes && "Invalid number of bytes in node XOR ID");
}

//-----------------------------------------------------------------------------

RoutingTable::RoutingTable(const xor_id&      node_id_xor) : id(node_id_xor) {
    // Buckets and mutexes are statically sized (std::array), no dynamic allocation needed.
}

//-----------------------------------------------------------------------------

// Add a new node to the routing table
bool RoutingTable::add_node(std::unique_ptr<Node> node) {
    if(!node) return false;

    auto node_id = hex_string_to_node_id(node->get_id());
    const int bucket_index = xor_distance_bit_index(this->id, node_id);
     
    if (node_id == this->id) {
        log_error("add_node: cannot add own node to routing table");
        return false;
    }

    if (bucket_index < 0 || bucket_index >= static_cast<int>(buckets.size())) {
        return false;
    }

    std::unique_lock lock(bucket_mutexes[bucket_index]);
    Bucket& bucket = buckets[bucket_index];

    // Check for duplicates
    for (const auto& existing_node : bucket) {
        if (existing_node->get_id() == node->get_id()) {
            log_warn("add_node: {} already exists in the routing table", node->get_i2p_address());
            return false; // already exists
        }
    }
    
    // Check if bucket is full
    if (bucket.size() >= NEROSHOP_DHT_NODES_PER_BUCKET) {
        // Optional: implement bucket splitting or LRU replacement
        log_error("add_node: bucket {} is full", bucket_index);
        return false; // Reject for now
    }

    // Get the i2p address *before* moving the shared_ptr
    std::string i2p_address = node->get_i2p_address();
    bucket.emplace_back(std::move(node));
    // Confirm node was added and print bucket size
    log_info("{}{} added to routing table (bucket: {}, size: {}){}", 
        "\033[1;32m", i2p_address, bucket_index, bucket.size(), color_reset);
    // After adding, print the use count
    if(!bucket.empty()) {
        std::shared_ptr<Node>& added_node = bucket.back();
        log_trace("add_node: Reference count after adding: {}", added_node.use_count());
    }
    return true;
}

//-----------------------------------------------------------------------------

bool RoutingTable::remove_node(const std::string& node_addr, uint16_t node_port) {
    for (int i = 0; i < 256; ++i) {
        std::unique_lock lock(bucket_mutexes[i]);
        auto& bucket = buckets[i];
        
        log_debug("remove_node: Scanning bucket {} (size: {})", i, bucket.size());
        
        auto it = std::remove_if(bucket.begin(), bucket.end(),
            [&](const std::shared_ptr<Node>& node) {
                if (!node) return false; // safety check
                const std::string& node_address = node->get_i2p_address();
                log_trace("remove_node: Comparing: [{}] vs [{}]", node_address, node_addr);
                return node_address == node_addr;
            });

        if (it != bucket.end()) {
            size_t removed = std::distance(it, bucket.end());
            bucket.erase(it, bucket.end());
            log_info("{}{} removed from routing table (bucket: {}, removed: {}, remaining: {}){}",
                "\033[1;91m", node_addr, i, removed, bucket.size(), color_reset);
            return true;
        }
    }

    log_warn("remove_node: {} not found in any bucket", node_addr);
    return false;
}

//-----------------------------------------------------------------------------

bool RoutingTable::remove_node(const std::string& node_id_hex) { // faster since it doesn't loop through all 256 buckets
    xor_id node_id;
    try {
        node_id = hex_string_to_node_id(node_id_hex);
    } catch (const std::exception& e) {
        log_error("remove_node: Invalid node ID: {}", e.what());
        return false;
    }

    int bucket_index = xor_distance_bit_index(id, node_id);

    std::unique_lock lock(bucket_mutexes[bucket_index]);
    auto& bucket = buckets[bucket_index];

    for (auto it = bucket.begin(); it != bucket.end(); /* no increment */) {
        if ((*it)->get_id() == node_id_hex) {
            std::weak_ptr<Node> weak_node = *it; // no ref count increment
            ////log_trace("remove_node: Reference count before deletion: {}", weak_node.use_count());
            it = bucket.erase(it); // erase returns the next iterator
            std::string i2p_address;
            if (auto node = weak_node.lock()) {
                i2p_address = node->get_i2p_address();
                ////log_trace("remove_node: Reference count after lock: {}", weak_node.use_count());
            } // shared_ptr `node` is destroyed here — ref count is decremented
            log_info("{}{} removed from routing table (bucket: {}, removed: 1, remaining: {}){}", "\033[1;91m", i2p_address, bucket_index, bucket.size(), color_reset);//log_info("{}Node with ID {} removed from routing table (bucket: {}, size: {}){}", "\033[91m", node_id_hex, bucket_index, bucket.size(), color_reset);
            log_trace("remove_node: Reference count after removal: {}", weak_node.use_count());
            return true;
        } else {
            ++it;
        }
    }

    log_warn("remove_node: Node with ID {} not found in bucket {}", node_id_hex, bucket_index);
    return false;
}

//-----------------------------------------------------------------------------

std::vector<std::weak_ptr<Node>> RoutingTable::find_closest_nodes(const std::string& key, int count) {
    xor_id target_id = hex_string_to_node_id(key); // will trigger "xor_distance_bit_index: Identical IDs" if this node is using itself as the target to find other nodes

    struct ScoredNode {
        std::shared_ptr<Node> node;
        uint32_t distance;
        bool operator<(const ScoredNode& other) const {
            return distance < other.distance;
        }
    };

    std::vector<ScoredNode> scored_nodes;

    for (int i = 0; i < 256; ++i) {
        std::shared_lock lock(bucket_mutexes[i]);
        for (const auto& node_ptr : buckets[i]) {
            if (!node_ptr) continue;
            uint32_t dist = xor_distance_bit_index(target_id, hex_string_to_node_id(node_ptr->get_id()));
            if (dist == 256) continue; // Skip if distance is 256 (to prevent node from returning itself in the find_node response)
            scored_nodes.push_back({ node_ptr, dist });
        }
    }

    std::sort(scored_nodes.begin(), scored_nodes.end());

    std::vector<std::weak_ptr<Node>> result;
    for (int i = 0; i < std::min(count, static_cast<int>(scored_nodes.size())); ++i) {
        result.push_back(scored_nodes[i].node);
    }

    return result;
}

//-----------------------------------------------------------------------------

std::weak_ptr<Node> RoutingTable::get_node_by_id(const std::string& node_id_hex) const {
    xor_id node_id = hex_string_to_node_id(node_id_hex);
    int bucket_index = xor_distance_bit_index(id, node_id);

    std::shared_lock lock(bucket_mutexes[bucket_index]);
    for (const auto& node_ptr : buckets[bucket_index]) {
        if (node_ptr && node_ptr->get_id() == node_id_hex) {
            return node_ptr; // implicit conversion to weak_ptr
        }
    }
    
    log_trace("get_node_by_id: Node with ID {} not found", node_id_hex); // <-- just to give the caller a heads up
    return std::weak_ptr<Node>(); // empty (expired)
}

//-----------------------------------------------------------------------------

int RoutingTable::get_bucket_index(const std::string& node_id_hex) const {
    xor_id node_id = hex_string_to_node_id(node_id_hex);
    int bucket_index = xor_distance_bit_index(id, node_id);
    return bucket_index;
}

//-----------------------------------------------------------------------------

int RoutingTable::get_bucket_index(const std::string& lhs, const std::string& rhs) {
    xor_id lhs_node_id = hex_string_to_node_id(lhs);
    xor_id rhs_node_id = hex_string_to_node_id(rhs);
    int bucket_index = xor_distance_bit_index(lhs_node_id, rhs_node_id);
    return bucket_index;
}

//-----------------------------------------------------------------------------

int RoutingTable::get_bucket_count() const {
    return static_cast<int>(buckets.size()); // always 256
}

//-----------------------------------------------------------------------------

int RoutingTable::get_node_count() const {
    int count = 0;
    for (int i = 0; i < 256; ++i) {
        std::shared_lock lock(bucket_mutexes[i]);
        count += buckets[i].size();
    }
    return count;
}

//-----------------------------------------------------------------------------

int RoutingTable::get_node_count(int bucket_index) const {
    assert(bucket_index >= 0 && bucket_index < 256);
    std::shared_lock lock(bucket_mutexes[bucket_index]);
    return buckets[bucket_index].size();
}

//-----------------------------------------------------------------------------

bool RoutingTable::is_bucket_full(int bucket_index) const {
    assert(bucket_index >= 0 && bucket_index < 256);
    std::shared_lock lock(bucket_mutexes[bucket_index]);
    return buckets[bucket_index].size() >= NEROSHOP_DHT_NODES_PER_BUCKET;
}

//-----------------------------------------------------------------------------

bool RoutingTable::are_buckets_full() const {
    for (int i = 0; i < 256; ++i) {
        if (!is_bucket_full(i)) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------

bool RoutingTable::has_node(const std::string& i2p_address, uint16_t port) {
    for (int i = 0; i < 256; ++i) {
        std::shared_lock lock(bucket_mutexes[i]);
        for (const auto& node : buckets[i]) {
            if (!node) continue;
            const std::string& node_address = node->get_i2p_address();
            if (node_address == i2p_address) {
                ////log_debug("has_node: {} found in bucket[{}]", i2p_address, i);
                return true;
            }
        }
    }
    return false;
}

//-----------------------------------------------------------------------------

bool RoutingTable::has_node(const std::string& node_id_hex) { // faster since it doesn't loop through all 256 buckets
    xor_id node_id = hex_string_to_node_id(node_id_hex);
    int bucket_index = xor_distance_bit_index(id, node_id);

    std::shared_lock lock(bucket_mutexes[bucket_index]);
    for (const auto& node : buckets[bucket_index]) {
        if (!node) continue;
        if (node->get_id() == node_id_hex) {
            ////log_debug("has_node: Node with ID {} found in bucket[{}]", node_id_hex, bucket_index);
            return true;
        }
    }
    return false;
}

//-----------------------------------------------------------------------------

void RoutingTable::print_table() const {
    for (int i = 0; i < 256; ++i) {
        std::shared_lock lock(bucket_mutexes[i]);
        if (!buckets[i].empty()) {
            std::cout << "Bucket " << i << ": ";
            for (const auto& node : buckets[i]) {
                if (node) {
                    std::cout << node->get_i2p_address() << " ";
                }
            }
            std::cout << "\n";
        }
    }
}

//-----------------------------------------------------------------------------

} // namespace neroshop
