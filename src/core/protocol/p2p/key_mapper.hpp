#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility> // std::pair
#include <shared_mutex>

namespace neroshop {

class KeyMapper { // maps search terms to DHT keys
public:
    KeyMapper() = default;
    ~KeyMapper();

    void add(const std::string& key, const std::string& value); // must be JSON value
    void sync(); // syncs mapping data to local database
    std::pair<std::string, std::string> serialize(); // Converts mapping data to JSON format
    
    std::vector<std::string> search_product_by_name(const std::string& product_name);//std::vector<std::string> search_user_by_id(const std::string& );//std::vector<std::string> search_order_by_id(const std::string& );
private:
    mutable std::shared_mutex data_mutex;
        
    std::unordered_map<std::string, std::vector<std::string>> product_ids;
    std::unordered_map<std::string, std::vector<std::string>> product_names; // maps a product name to a list of corresponding listing keys
    std::unordered_map<std::string, std::vector<std::string>> product_categories;
    std::unordered_map<std::string, std::vector<std::string>> product_tags;
    std::unordered_map<std::string, std::vector<std::string>> product_codes;
    std::unordered_map<std::string, std::vector<std::string>> listing_ids;
    std::unordered_map<std::string, std::vector<std::string>> listing_locations;
    std::unordered_map<std::string, std::vector<std::string>> seller_ids;
    std::unordered_map<std::string, std::vector<std::string>> user_ids; // maps a monero address (ID) to the corresponding account key
    std::unordered_map<std::string, std::vector<std::string>> display_names; // maps a display name to a list of corresponding account keys
    std::unordered_map<std::string, std::vector<std::string>> order_ids; // maps a order uuid to the corresponding order key.
    std::unordered_map<std::string, std::vector<std::string>> order_recipients;
    std::unordered_map<std::string, std::vector<std::string>> product_ratings;
    std::unordered_map<std::string, std::vector<std::string>> seller_ratings;
    std::unordered_map<std::string, std::vector<std::string>> messages;
};

}
