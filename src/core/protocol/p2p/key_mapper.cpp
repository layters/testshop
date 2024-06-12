#include "key_mapper.hpp"

#include <nlohmann/json.hpp>

#include "../../tools/string.hpp"
#include "../../database/database.hpp"
#include "../../crypto/sha3.hpp"

namespace neroshop_string = neroshop::string;

namespace neroshop {
//-----------------------------------------------------------------------------

KeyMapper::~KeyMapper() {
    product_ids.clear();
    product_names.clear();
    product_categories.clear();
    product_tags.clear();
    product_codes.clear();
    listing_ids.clear();
    listing_locations.clear();
    seller_ids.clear();
    user_ids.clear();
    display_names.clear();
    order_ids.clear();
    order_recipients.clear();
    product_ratings.clear();
    seller_ratings.clear();
    messages.clear();
}

//-----------------------------------------------------------------------------

void KeyMapper::add(const std::string& key, const std::string& value) {
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(value);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return;
    }
    
    std::string metadata = json["metadata"].get<std::string>();

    //-----------------------------------------------
    // Note: As long as we have the user id, we can find the user
    if(metadata == "user") {
        // Store user-related data in the user_ids and display_names unordered maps
        if (json.contains("monero_address") && json["monero_address"].is_string()) {
            std::string user_id = json["monero_address"].get<std::string>();
            user_ids[user_id].push_back(key);//std::cout << "user_id (" << user_id << ") has been mapped to account key (" << key << ")\n";
        }
        if (json.contains("display_name") && json["display_name"].is_string()) {
            std::string display_name = json["display_name"].get<std::string>();
            display_names[display_name].push_back(key);//std::cout << "display_name (" << display_name << ") has been mapped to account key (" << key << ")\n";
        }
    }
    //-----------------------------------------------
    if(metadata == "listing") {
        // Note: As long as we have the product id, we can find the product_ratings
        // Map a listing's key to product_ids, product_names, product_categories, product_tags, and product_codes
        if(json.contains("product") && json["product"].is_object()) {
            nlohmann::json product_obj = json["product"];
            if (product_obj.contains("id") && product_obj["id"].is_string()) {
                std::string product_id = product_obj["id"].get<std::string>();
                product_ids[product_id].push_back(key);
            }        
            if (product_obj.contains("name") && product_obj["name"].is_string()) {
                std::string product_name = product_obj["name"].get<std::string>();
                product_names[product_name].push_back(key);
            }
            if (product_obj.contains("category") && product_obj["category"].is_string()) {
                std::string product_category = product_obj["category"].get<std::string>();
                product_categories[product_category].push_back(key);
            }
            if (product_obj.contains("subcategories") && product_obj["subcategories"].is_array()) {
                const auto& subcategories = product_obj["subcategories"];
                for (const auto& subcategory : subcategories) {
                    if (subcategory.is_string()) {
                        std::string product_subcategory = subcategory.get<std::string>();
                        product_categories[product_subcategory].push_back(key);
                    }
                }
            }        
            if (product_obj.contains("tags") && product_obj["tags"].is_array()) {
                const auto& tags = product_obj["tags"];
                for (const auto& tag : tags) {
                    if (tag.is_string()) {
                        std::string product_tag = tag.get<std::string>();
                        product_tags[product_tag].push_back(key);
                    }
                }
            }
            if (product_obj.contains("code") && product_obj["code"].is_string()) {
                std::string product_code = product_obj["code"].get<std::string>();
                product_codes[product_code].push_back(key);
            }
        }
        // Map a listing's key to listing_id
        if (json.contains("id") && json["id"].is_string()) {
            std::string listing_id = json["id"].get<std::string>();
            listing_ids[listing_id].push_back(key);
        }        
        // Map a listing's key to seller_id
        if (json.contains("seller_id") && json["seller_id"].is_string()) {
            std::string seller_id = json["seller_id"].get<std::string>();
            seller_ids[seller_id].push_back(key);
        }
        // Map a listing's key to a listing_location
        if (json.contains("location") && json["location"].is_string()) {
            std::string location = json["location"].get<std::string>();
            listing_locations[location].push_back(key);
        }
    }
    //-----------------------------------------------    
    if(metadata == "order") {
        // Store order-related data in the order_ids unordered map
        if (json.contains("id") && json["id"].is_string()) {
            std::string order_id = json["id"].get<std::string>();
            order_ids[order_id].push_back(key);
        }
        // Map seller_ids to orders
        if(json.contains("items") && json["items"].is_array()) {
            const auto& order_items = json["items"];
            
            for (const auto& item : order_items) {
                if(item.is_object()) {
                    if (item.contains("seller_id") && item["seller_id"].is_string()) {
                        std::string seller_id = item["seller_id"].get<std::string>();
                        order_recipients[seller_id].push_back(key);
                    }
                }
            }
        }
    }    
    //-----------------------------------------------
    if(metadata == "product_rating") {
        // Map a product_rating's key to product_id
        if (json.contains("product_id") && json["product_id"].is_string()) {
            std::string product_id = json["product_id"].get<std::string>();
            product_ratings[product_id].push_back(key);
        }
    }
    //-----------------------------------------------
    if(metadata == "seller_rating") {
        // Map a seller_rating's key to seller_id
        if (json.contains("seller_id") && json["seller_id"].is_string()) {
            std::string seller_id = json["seller_id"].get<std::string>();
            seller_ratings[seller_id].push_back(key);
        }    
    }
    //-----------------------------------------------
    if(metadata == "message") {
        // Map a message's key to a recipient_id
        if (json.contains("recipient_id") && json["recipient_id"].is_string()) {
            std::string recipient_id = json["recipient_id"].get<std::string>();
            messages[recipient_id].push_back(key);
        }
    }
    //-----------------------------------------------
    sync(); // Sync to database
}

void KeyMapper::sync() {
    db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    //database->execute("BEGIN TRANSACTION;");
    //-----------------------------------------------
    // Insert data from 'user_ids'
    for (const auto& entry : user_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "user";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    // Insert data from 'display_names'
    for (const auto& entry : display_names) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "user";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }    
    //-----------------------------------------------
    // Insert data from 'product_ids'
    for (const auto& entry : product_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    // Insert data from 'product_names'
    for (const auto& entry : product_names) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    // Insert data from 'product_categories'
    for (const auto& entry : product_categories) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    // Insert data from 'product_tags'
    for (const auto& entry : product_tags) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }        
    // Insert data from 'product_codes'
    for (const auto& entry : product_codes) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    // Insert data from 'listing_ids'
    for (const auto& entry : listing_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }    
    // Insert data from 'listing_locations'
    for (const auto& entry : listing_locations) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }        
    // Insert data from 'seller_ids`
    for (const auto& entry : seller_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "listing";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    //-----------------------------------------------
    // Insert data from 'order_ids'
    for (const auto& entry : order_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "order";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    // Insert data from 'order_recipients'
    for (const auto& entry : order_recipients) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "order";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    //-----------------------------------------------
    // Insert data from 'product_ratings'
    for (const auto& entry : product_ratings) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "product_rating";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }    
    //-----------------------------------------------
    // Insert data from 'seller_ratings'
    for (const auto& entry : seller_ratings) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "seller_rating";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ? AND key = ?;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?, ?, ?);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }            
    //-----------------------------------------------
    // Insert data from 'messages'
    for (const auto& entry : messages) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;
        const std::string content = "message";

        for (const std::string& key : keys) {
            // Ignore any empty keys
            if(key.empty() || key.length() != 64) continue;
            // Check if the record already exists
            std::string select_query = "SELECT COUNT(*) FROM mappings WHERE search_term = ?1 AND key = ?2;";
            bool exists = database->get_integer_params(select_query, { search_term, key });
            
            // If no duplicate record found, perform insertion
            if(!exists) {
                std::string insert_query = "INSERT INTO mappings (search_term, key, content) VALUES (?1, ?2, ?3);";
                database->execute_params(insert_query, { search_term, key, content });
            }
        }
    }
    //-----------------------------------------------
    //database->execute("COMMIT;");
}

//-----------------------------------------------------------------------------

std::pair<std::string, std::string> KeyMapper::serialize() { // no longer in use
    nlohmann::json data;
    //-----------------------------------------------
    // Add user_ids
    for (const auto& entry : user_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json user_object;
        for (const auto& key : keys) {
            user_object.push_back(key);
        }

        data["user_id"][search_term] = user_object;
    }    
    // Add display_names
    for (const auto& entry : display_names) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json display_name_object;
        for (const auto& key : keys) {
            display_name_object.push_back(key);
        }

        data["display_name"][search_term] = display_name_object;
    }        
    //-----------------------------------------------
    // Add product_ids
    for (const auto& entry : product_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json product_id_object;
        for (const auto& key : keys) {
            product_id_object.push_back(key);
        }

        data["product_id"][search_term] = product_id_object;
    }
    // Add product_names
    for (const auto& entry : product_names) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json product_name_object;
        for (const auto& key : keys) {
            product_name_object.push_back(key);
        }

        data["product_name"][search_term] = product_name_object;
    }
    // Add product_categories
    for (const auto& entry : product_categories) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json product_category_object;
        for (const auto& key : keys) {
            product_category_object.push_back(key);
        }

        data["product_category"][search_term] = product_category_object;
    }
    // Add product_tags
    for (const auto& entry : product_tags) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json product_tag_object;
        for (const auto& key : keys) {
            product_tag_object.push_back(key);
        }

        data["product_tag"][search_term] = product_tag_object;
    }
    // Add product_codes
    for (const auto& entry : product_codes) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json product_code_object;
        for (const auto& key : keys) {
            product_code_object.push_back(key);
        }

        data["product_code"][search_term] = product_code_object;
    }
    // Add listing_ids
    for (const auto& entry : listing_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json listing_id_object;
        for (const auto& key : keys) {
            listing_id_object.push_back(key);
        }

        data["listing_id"][search_term] = listing_id_object;
    }    
    // Add listing_locations
    for (const auto& entry : listing_locations) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json product_sale_location_object;
        for (const auto& key : keys) {
            product_sale_location_object.push_back(key);
        }

        data["product_location"][search_term] = product_sale_location_object;
    }
    // Add seller_ids
    for (const auto& entry : seller_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json seller_id_object;
        for (const auto& key : keys) {
            seller_id_object.push_back(key);
        }

        data["seller_id"][search_term] = seller_id_object;
    }    
    //-----------------------------------------------
    // Add order_ids
    for (const auto& entry : order_ids) {
        const std::string& search_term = entry.first;
        const std::vector<std::string>& keys = entry.second;

        nlohmann::json order_id_object;
        for (const auto& key : keys) {
            order_id_object.push_back(key);
        }

        data["order_id"][search_term] = order_id_object;
    }  
    //-----------------------------------------------
    data["metadata"] = "index";
    
    std::string value = data.dump();
    std::string key = neroshop::crypto::sha3_256(value);
    
    #ifdef NEROSHOP_DEBUG
    std::cout << "Index generated:\n";
    std::cout << data.dump(4) << "\n";
    #endif
    // Return key-value pair
    return std::make_pair(key, value);
}

//-----------------------------------------------------------------------------

std::vector<std::string> KeyMapper::search_product_by_name(const std::string& product_name) {
    std::vector<std::string> matching_keys;

    std::string product_name_lower = neroshop_string::lower(product_name);

    for (const auto& entry : product_names) {
        std::string entry_lower = neroshop_string::lower(entry.first);
        if (entry_lower.find(product_name_lower, 0) == 0) { // starts with the search term (ex. for Banana, "b", "ba", "ban", "bana", "banan", or "banana" should work)
            matching_keys.insert(matching_keys.end(), entry.second.begin(), entry.second.end());
        } else {
            if(neroshop_string::contains(entry_lower, product_name_lower)) { // contains a substring that matches in the same order anywhere within the string
                matching_keys.insert(matching_keys.end(), entry.second.begin(), entry.second.end());
            }
        }
    }

    return matching_keys;
}

//-----------------------------------------------------------------------------
}

/*int main() {
    KeyMapper::product_names.insert(std::make_pair("Apple", std::vector<std::string>{"9341dd5ebbe0d457e1306bdb68f2cd13a0d3bac4e582012ae71c2bce47f8bb91"}));
    KeyMapper::product_names.insert(std::make_pair("Banana", std::vector<std::string>{"8063ed324ad571c0278a1d5d11b5d620a41e605d389e2ad7f268c196f7411035"}));
    KeyMapper::product_names.emplace(std::make_pair("Cherry", std::vector<std::string>{"f246efebbca8d633d2d9ce15ffd0ffeb6aabeddf19cdc060fe348c282e371b7a"}));
    KeyMapper::product_names.insert(std::make_pair("Watermelon", std::vector<std::string>{"0fe3907f0c4012aa9967e2dac81ef31c008ebf2b58f36e107c0c9bd61ba9d53e"}));
    //KeyMapper::product_names.insert(std::make_pair("", std::vector<std::string>{""}));
    std::cout << "Number of products: " << KeyMapper::product_names.size() << "\n";
    auto products = KeyMapper::search_product_by_name("er");//("ana");//("app");//"ban");
    for(const auto& name : products) {
        std::cout << name << "\n";
    }
}*/ // g++ indexer.cpp product.cpp -o index
