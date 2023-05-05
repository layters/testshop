#include "serializer.hpp"

#include "crypto/sha256.hpp"//#include "../../crypto/sha256.hpp"
#include "tools/string.hpp"

#include <nlohmann/json.hpp>

/** Thoughts and ideas: I think cart and wishlist data should be stored locally in the database file:
    1. Data privacy: By storing cart and wishlist data locally, you can ensure that sensitive information is not shared across the network or exposed to unauthorized users. This can help to protect users' privacy and ensure that their data is secure.
    2. Performance: Retrieving data from a local database file can be faster than accessing data stored in a DHT, since it does not require network communication. This can help to improve the performance of your application and provide a better user experience.
    3. Ease of implementation: Storing data locally in a database file can be easier to implement and maintain than using a DHT. You can use a variety of database technologies and libraries to store and manage your data, and you may be able to take advantage of existing tools and frameworks to simplify development.
**/

namespace neroshop_crypto = neroshop::crypto;
namespace neroshop_string = neroshop::string;

std::pair<std::string, std::string> neroshop::Serializer::serialize(const neroshop::Object& obj) {
    // Determine namespace
    std::string name_prefix = "";
    nlohmann::json json_object = {};
    //-------------------------------------------------------------
    if(std::holds_alternative<Seller>(obj)) {
        name_prefix = "seller";
        const Seller& seller = std::get<Seller>(obj);
        json_object["display_name"] = seller.get_name();
        json_object["monero_address"] = seller.get_id(); // used as identifier
        std::string public_key = seller.get_public_key();
        if(!public_key.empty()) json_object["public_key"] = public_key;
        //json_object["avatar"] = ; // TODO: Avatars
    }
    
    if(std::holds_alternative<Product>(obj)) {
        name_prefix = "product";
        const Product& product = std::get<Product>(obj);
        json_object["id"] = product.get_id();
        json_object["name"] = product.get_name();
        json_object["description"] = product.get_description();
        for (const auto& attr : product.get_attributes()) {
            nlohmann::json attribute_obj = {};
            if (!attr.color.empty()) attribute_obj["color"] = attr.color;
            if (!attr.size.empty()) attribute_obj["size"] = attr.size;
            if (attr.weight != 0.0) attribute_obj["weight"] = attr.weight;
            if (!attr.material.empty()) attribute_obj["material"] = attr.material;
            bool dimensions_empty = (std::get<0>(attr.dimensions) == 0.0 && std::get<1>(attr.dimensions) == 0.0 && std::get<2>(attr.dimensions) == 0.0) || (std::get<3>(attr.dimensions).empty()); // make sure either all values in the dimensions tuple contains non-zero values or a non-empty string
            if (!dimensions_empty) {
                std::string format = neroshop_string::lower(std::get<3>(attr.dimensions));
                format.erase(std::remove_if(format.begin(), format.end(), [](char c) { return c == 'x' || c == '-' || c == ' '; }), format.end());
                nlohmann::json dimensions_obj = {};
                if(format == "dh") {
                    dimensions_obj["diameter"] = std::get<0>(attr.dimensions);
                    dimensions_obj["height"] = std::get<1>(attr.dimensions);
                }                
                if(format == "wdh") {
                    dimensions_obj["width"] = std::get<0>(attr.dimensions);
                    dimensions_obj["depth"] = std::get<1>(attr.dimensions);
                    dimensions_obj["height"] = std::get<2>(attr.dimensions);
                }
                if(format == "lwh") {
                    if(std::get<0>(attr.dimensions) != 0.0) dimensions_obj["length"] = std::get<0>(attr.dimensions);
                    dimensions_obj["width"] = std::get<1>(attr.dimensions);
                    if(std::get<2>(attr.dimensions) != 0.0) dimensions_obj["height"] = std::get<2>(attr.dimensions);
                }
                attribute_obj["dimensions"] = dimensions_obj;
            }
            if (!attr.brand.empty()) attribute_obj["brand"] = attr.brand;
            if (!attr.model.empty()) attribute_obj["model"] = attr.model;
            if (!attr.manufacturer.empty()) attribute_obj["manufacturer"] = attr.manufacturer;
            if (!attr.country_of_origin.empty()) attribute_obj["country_of_origin"] = attr.country_of_origin;
            if (!attr.warranty_information.empty()) attribute_obj["warranty_information"] = attr.warranty_information;
            if (!attr.product_code.empty()) attribute_obj["product_code"] = attr.product_code;
            if (!attr.style.empty()) attribute_obj["style"] = attr.style;
            if (!attr.gender.empty()) attribute_obj["gender"] = attr.gender;
            if (attr.age_range.second > 0) {
                nlohmann::json age_range_obj = {};
                age_range_obj["min"] = attr.age_range.first;
                age_range_obj["max"] = attr.age_range.second;
                attribute_obj["age_range"] = age_range_obj;
            }
            if (!attr.energy_efficiency_rating.empty()) attribute_obj["energy_efficiency_rating"] = attr.energy_efficiency_rating;
            if (!attr.safety_features.empty()) {
                nlohmann::json safety_features_array = nlohmann::json::array();
                for (const auto& feature : attr.safety_features) {
                    safety_features_array.push_back(feature);
                }
                attribute_obj["safety_features"] = safety_features_array;
            }
            if (attr.quantity_per_package != 0) attribute_obj["quantity_per_package"] = attr.quantity_per_package;
            if (!attr.release_date.empty()) attribute_obj["release_date"] = attr.release_date;
            json_object["attributes"].push_back(attribute_obj); // rename to "variants" or "variant_options"?
        }
        std::string product_code = product.get_code();
        if(!product_code.empty()) json_object["code"] = product_code; // can be left empty esp. if variants have their own product codes
        json_object["category_id"] = product.get_category_id(); // TODO: just use a category string
        std::vector<std::string> tags = product.get_tags();
        if (!tags.empty()) {
            nlohmann::json tags_array = {};
            for (const auto& tag : tags) {
                tags_array.push_back(tag);
            }
            json_object["tags"] = tags_array;
        }
        //json_object["images"] = // TODO: Product Images
    }
    
    if(std::holds_alternative<Listing>(obj)) {
        name_prefix = "listing";
        const Listing& listing = std::get<Listing>(obj);
        json_object["id"] = listing.get_id();
        json_object["product_id"] = listing.get_product_id();
        json_object["seller_id"] = listing.get_seller_id();
        json_object["quantity"] = listing.get_quantity();
        json_object["price"] = listing.get_price();
        json_object["currency"] = listing.get_currency();
        json_object["condition"] = listing.get_condition();
        json_object["location"] = listing.get_location();
        json_object["date"] = listing.get_date();
    }
    
    if(std::holds_alternative<Cart>(obj)) { 
        name_prefix = "cart";
        const Cart& cart = std::get<Cart>(obj);
        json_object["id"] = cart.get_id();
        json_object["owner_id"] = cart.get_owner_id();
        for (const auto& pair : cart.contents) {
            nlohmann::json content_obj = {};
            content_obj[pair.first] = pair.second; // <product_id>:<quantity>
            json_object["contents"].push_back(content_obj); // cart_items // TODO: encrypt contents
        }
    }
    
    if(std::holds_alternative<Order>(obj)) {
        name_prefix = "order";
        const Order& order = std::get<Order>(obj);
        json_object["id"] = order.get_id();
        json_object["created_at"] = order.get_date();
        json_object["status"] = order.get_status_as_string();
        json_object["customer_id"] = order.get_customer_id();
        json_object["subtotal"] = order.get_subtotal();
        json_object["discount"] = order.get_discount();
        json_object["shipping_cost"] = order.get_shipping_cost();
        json_object["total"] = order.get_total();
        json_object["payment_option"] = order.get_payment_option_as_string();
        if(order.get_payment_coin() != PaymentCoin::None) json_object["coin"] = order.get_payment_coin_as_string();
        json_object["delivery_option"] = order.get_delivery_option_as_string();
        json_object["notes"] = order.get_notes(); // TODO: encrypt notes
        for(const auto& item : order.get_items()) {
            nlohmann::json order_item_obj = {};
            order_item_obj["product_id"] = std::get<0>(item);
            order_item_obj["quantity"] = std::get<1>(item);
            order_item_obj["seller_id"] = std::get<2>(item);
            json_object["items"].push_back(order_item_obj); // order_items // TODO: encrypt items
        }
    }
    
    if(std::holds_alternative<ProductRating>(obj)) {
        name_prefix = "product_rating";
        const ProductRating& product_rating = std::get<ProductRating>(obj);
        json_object["product_id"] = product_rating.product_id;
        json_object["stars"] = product_rating.stars;
        json_object["rater_id"] = product_rating.rater_id;
        json_object["comments"] = product_rating.comments;
        json_object["signature"] = product_rating.signature;
    }
    
    if(std::holds_alternative<SellerRating>(obj)) {
        name_prefix = "seller_rating";
        const SellerRating& seller_rating = std::get<SellerRating>(obj);
        json_object["seller_id"] = seller_rating.seller_id;
        json_object["score"] = seller_rating.score;
        json_object["rater_id"] = seller_rating.rater_id;
        json_object["comments"] = seller_rating.comments;
        json_object["signature"] = seller_rating.signature;
    }    
    // TODO: Favorites (wishlists)
    //-------------------------------------------------------------
    assert(!name_prefix.empty() && "Invalid neroshop object type");
    // Get the `value`
    std::string value = json_object.dump();
    // Convert the string to a byte string (UTF-8 encoding) in case we ever decide to store values as bytes instead of as json string
    std::vector<uint8_t> value_bytes = nlohmann::json::to_msgpack(json_object);
    // Generate `key` by hashing the value
    std::string key = neroshop_crypto::sha256(value);
    //-------------------------------------------------------------
    #ifdef NEROSHOP_DEBUG
    std::cout << json_object.dump(4) << "\n";
    
    std::cout << "\nkey (hash of value): " << key << "\n";
    std::cout << "value: " << value << "\n";
    std::cout << "bytes (to_msgpack):\n";
    for (auto byte : value_bytes) {
        std::cout << std::hex << (int)byte << " "; // A hex string is simply a way of representing data in hexadecimal format, which is commonly used for representing binary data in a human-readable form.
    }
    std::cout << "\njson (from msgpack):\n";
    nlohmann::json bytes_to_json = nlohmann::json::from_msgpack(value_bytes);
    std::cout << bytes_to_json << "\n";
    assert(value == bytes_to_json.dump()); // The value should be the same as the bytes dumped
    #endif
    //-------------------------------------------------------------
    // Return key-value pair
    return std::make_pair(name_prefix + "_" + key, value);
}

// incomplete
/*std::shared_ptr<neroshop::Object> neroshop::Serializer::deserialize(const std::string& json_object) {//const Object& deserialize(const std::string& json_object);
    // Deserialize the JSON object and create the appropriate variant object
    //Object variant_object;// = create_variant_object(json_object);

    // Create a shared pointer to the variant object and return it
    //return std::make_shared<Object>(std::move(variant_object));//return variant_object; // or return a const reference to the variant object
}*/

template <typename... Args>
std::string neroshop::Serializer::to_json(Args&&... args) {
    nlohmann::json json_object = nlohmann::json::object({std::forward<Args>(args)...});
    return json_object.dump();
}

/*int main() {
    std::cout << neroshop::Serializer::to_json(std::make_pair("key1", 123), 
        std::make_pair("key2", "value2"), 
        std::make_pair("key3", true)) << "\n";
    std::cout << "\n";


    neroshop::Object obj = neroshop::Product { "my_product_uuid", "Nintendo Switch Lite", "A really fun gaming system", 
        {
            {.color = "yellow"   , .size = "lite", .weight = .61, .product_code = "00045496882303"}, // 0.61 lbs
            {.color = "gray"     , .size = "lite", .weight = .61, .product_code = "00045496882280"},
            {.color = "turquoise", .size = "lite", .weight = .61, .product_code = "00045496882266"},
            {.color = "coral"    , .size = "lite", .weight = .61, .product_code = "00045496882662"},
            {.color = "blue"     , .size = "lite", .weight = .61, .product_code = "00045496882716"},
        },
        "my_product_code",
        0,
        {"video games", "nintendo", "switch lite"}
    };
    
    //neroshop::Object obj = neroshop::ProductRating { "user1", "Good product", "signed", "product123", 4 };
    
    //neroshop::SellerRating sr = { "user2", "Fast shipping", "signed", "seller456", 1 };
    
    std::pair<std::string, std::string> result = neroshop::Serializer::serialize(obj); // pass the variant to the serialize function
    std::cout << "\n\nresult\n";
    std::cout << result.first << ": " << result.second << "\n";
    return 0;
}*/
// g++ serializer.cpp crypto/sha256.cpp tools/logger.cpp price/*.cpp config.cpp script.cpp cart.cpp product.cpp listing.cpp order.cpp database/database.cpp database/sqlite.cpp -o serialize -lcrypto -lssl -I../../external/json/single_include -Icrypto -I../../external/stduuid/include -I../../external/stduuid/ -I../../external/lua/src -L../../build -lsqlite3 -lcurl -llua  

