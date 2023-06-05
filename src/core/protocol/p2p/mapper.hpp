#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <utility>  // for std::pair

namespace neroshop {

class Mapper { // maps search terms to DHT keys
public:
    static std::unordered_map<std::string, std::vector<std::string>> user_accounts;
    static std::unordered_map<std::string, std::vector<std::string>> product_ids;
    static std::unordered_map<std::string, std::vector<std::string>> product_names; // maps a product name to a list of corresponding listing keys
    static std::unordered_map<std::string, std::vector<std::string>> product_categories;
    static std::unordered_map<std::string, std::vector<std::string>> product_tags;
    static std::unordered_map<std::string, std::vector<std::string>> product_codes;
    static std::unordered_map<std::string, std::vector<std::string>> product_sale_locations;
    static std::unordered_map<std::string, std::vector<std::string>> seller_ids;
    static std::unordered_map<std::string, std::vector<std::string>> user_ids; // maps a monero address (ID) to the corresponding node ID
    static std::unordered_map<std::string, std::vector<std::string>> display_names; // maps a display name to a list of corresponding node IDs
    static std::unordered_map<std::string, std::vector<std::string>> order_ids; // maps a order uuid to the corresponding order key.

    static void add(const std::string& key, const std::string& value); // must be JSON value
    static void sync(); // syncs mapping data to local database
    static std::pair<std::string, std::string> serialize(); // Converts mapping data to JSON format
    
    static std::vector<std::string> search_product_by_name(const std::string& product_name);//static std::vector<std::string> search_user_by_id(const std::string& );//static std::vector<std::string> search_order_by_id(const std::string& );
private:
};

}
