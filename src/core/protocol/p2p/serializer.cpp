#include "serializer.hpp"

#include "../../crypto/sha3.hpp"
#include "../../tools/string.hpp"
#include "../../tools/timestamp.hpp"
#include "../../wallet/wallet.hpp"
#include "../../settings.hpp"

#include <regex>
#include <cmath> // std::abs

#include <nlohmann/json.hpp>

namespace neroshop {
//-----------------------------------------------------------------------------

void set_expiration(nlohmann::json& json_object, const std::string& metadata) {
    if(metadata == "user" || metadata == "seller_rating") { return; } // Never expires
    
    nlohmann::json settings = nlohmann::json::parse(neroshop::load_json(), nullptr, false);
    if(settings.is_discarded()) {
        if(metadata == "order") {
            json_object["expiration_date"] = neroshop::timestamp::get_utc_timestamp_after_duration(2, "years"); // default: 2 years
        }
        if(metadata == "message") {
            json_object["expiration_date"] = neroshop::timestamp::get_utc_timestamp_after_duration(30, "days"); // default: 30 days
        }
    } else {
        std::string expires_in = settings["data_expiration"][metadata].get<std::string>();
        if(neroshop::string_tools::contains(expires_in, "Never") && 
            (metadata == "listing" || metadata == "product_rating")) { 
            return; 
        }
        
        std::regex pattern("(\\d+) (\\w+)"); // Regular expression to match number followed by text
        std::smatch match;
        if (!std::regex_search(expires_in, match, pattern)) {
            throw std::runtime_error("Malformed or invalid expiration (settings.json)");
        }
        
        if (std::regex_search(expires_in, match, pattern)) {
            std::string number_str = match[1].str();
            int number = std::stoi(number_str);

            std::string time_unit = match[2].str();
            if(!time_unit.empty() && time_unit.back() != 's') { time_unit.push_back('s'); }
            
            json_object["expiration_date"] = neroshop::timestamp::get_utc_timestamp_after_duration(number, time_unit);
        }
    }
}

//-----------------------------------------------------------------------------

std::pair<std::string, std::string/*std::vector<uint8_t>*/> Serializer::serialize(const neroshop::Object& obj) {
    nlohmann::json json_object = {};
    
    if(std::holds_alternative<Listing>(obj)) {
        const Listing& listing = std::get<Listing>(obj);
        json_object["id"] = listing.get_id();
        json_object["seller_id"] = listing.get_seller_id();
        json_object["quantity"] = std::abs(listing.get_quantity());
        json_object["price"] = std::abs(listing.get_price());
        json_object["currency"] = listing.get_currency();
        json_object["condition"] = listing.get_condition();
        std::string location = listing.get_location();
        if(!location.empty() && location != "Unspecified") {
            json_object["location"] = location;
        }
        json_object["date"] = listing.get_date();
        json_object["signature"] = listing.get_signature();// listings can be updated by the user that can verify the signature
        int quantity_per_order = listing.get_quantity_per_order();
        if(quantity_per_order > 0) {
            json_object["quantity_per_order"] = quantity_per_order;
        }
        auto payment_method = listing.get_payment_method(); // only a single payment method (PaymentMethod::Crypto) is supported
        nlohmann::json payment_methods_array = {};
        std::string payment_method_str = get_payment_method_as_string(payment_method);
        payment_methods_array.push_back(payment_method_str);
        json_object["payment_methods"] = payment_methods_array;
        auto payment_coins = listing.get_payment_coins();
        if(!payment_coins.empty()) {
            nlohmann::json payment_coins_array = {};
            for(const auto& payment_coin : payment_coins) {
                std::string payment_coin_str = get_payment_coin_as_string(payment_coin);
                payment_coins_array.push_back(payment_coin_str);
            }
            json_object["payment_coins"] = payment_coins_array;
        }
        auto payment_options = listing.get_payment_options();
        if(!payment_options.empty()) {
            nlohmann::json payment_options_array = {};
            for(const auto& payment_option : payment_options) {
                std::string payment_option_str = get_payment_option_as_string(payment_option);
                payment_options_array.push_back(payment_option_str);
            }
            json_object["payment_options"] = payment_options_array;
        }
        auto delivery_options = listing.get_delivery_options();
        if(!delivery_options.empty()) {
            nlohmann::json delivery_options_array = {};
            for(const auto& delivery_option : delivery_options) {
                std::string delivery_option_str = get_delivery_option_as_string(delivery_option);
                delivery_options_array.push_back(delivery_option_str);
            }
            json_object["delivery_options"] = delivery_options_array;
        }
        auto shipping_options = listing.get_shipping_options();
        if(!shipping_options.empty()) {
            nlohmann::json shipping_options_array = {};
            for(const auto& shipping_option : shipping_options) {
                std::string shipping_option_str = get_shipping_option_as_string(shipping_option);
                shipping_options_array.push_back(shipping_option_str);
            }
            json_object["shipping_options"] = shipping_options_array;
        }
        auto shipping_costs = listing.get_shipping_costs();
        if(!shipping_costs.empty()) {
            nlohmann::json shipping_costs_object;
            for (const auto& [shipping_option, cost] : shipping_costs) {
                std::string shipping_option_str = get_shipping_option_as_string(shipping_option);
                shipping_costs_object[shipping_option_str] = cost;
            }
            json_object["shipping_costs"] = shipping_costs_object;
        }
        auto custom_rates = listing.get_custom_rates();
        if(!custom_rates.empty()) {
            nlohmann::json custom_rates_object;
            for (const auto& [payment_coin, rate] : custom_rates) {
                std::string payment_coin_str = get_payment_coin_as_string(payment_coin);
                custom_rates_object[payment_coin_str] = rate;
            }
            json_object["custom_rates"] = custom_rates_object;
        }
        // Include the product serialization within the listing serialization
        assert(listing.get_product() != nullptr);
        const Product& product = *listing.get_product();
        nlohmann::json product_obj = {};
        ////product_obj["id"] = product.get_id(); // use listing_id instead
        product_obj["name"] = product.get_name();
        product_obj["description"] = product.get_description();
        double weight = product.get_weight();
        if(weight > 0.0) {
            product_obj["weight"] = weight;
        }
        // Collect the ordered list of option keys
        auto variant_options = product.get_options();
        if (!variant_options.empty()) {
            std::vector<std::string> option_keys;
            option_keys.reserve(variant_options.size());
            for (const auto& [option_name, _] : variant_options) {
                option_keys.push_back(option_name);
            }
            product_obj["options"] = option_keys;
            // Serialize variants array
            const auto& variants = product.get_variants();
            if (!variants.empty()) {
                nlohmann::json variants_array = nlohmann::json::array();
                variants_array.get_ref<nlohmann::json::array_t&>().reserve(variants.size());
                for (const auto& variant : variants) {
                    nlohmann::json variant_obj = {};
                    variant_obj["options"] = nlohmann::json::object();
                    for (const auto& [key, value] : variant.options) {
                        variant_obj["options"][key] = value;
                    }
                    // Optional fields — only serialize if present
                    if (!variant.info.condition.empty()) {
                        variant_obj["condition"] = variant.info.condition;
                    }
                    if (variant.info.weight.has_value() && variant.info.weight.value() > 0.0) {
                        variant_obj["weight"] = variant.info.weight.value();
                    }
                    if (variant.info.price.has_value()) { // can be zero, so no zero checks
                        variant_obj["price"] = std::abs(variant.info.price.value());
                    }
                    if (variant.info.quantity.has_value()) { // can be zero, so no zero checks
                        variant_obj["quantity"] = std::abs(variant.info.quantity.value());
                    }
                    if (!variant.info.product_code.empty()) {
                        variant_obj["product_code"] = variant.info.product_code;
                    }
                    if (variant.info.image_index.has_value() && variant.info.image_index.value() > -1) {
                        variant_obj["image_index"] = variant.info.image_index.value();
                    }

                    variants_array.push_back(variant_obj);
                }
                product_obj["variants"] = variants_array;
            }
        }
        std::string product_code = product.get_code();
        if(!product_code.empty()) product_obj["code"] = product_code; // can be left empty esp. if variants have their own product codes
        product_obj["category"] = product.get_category_as_string();
        std::set<std::string> subcategories = product.get_subcategories_as_string();
        if(!subcategories.empty()) {
            nlohmann::json subcategory_array = {};
            for (const auto& subcategory : subcategories) {
                subcategory_array.push_back(subcategory);
            }
            product_obj["subcategories"] = subcategory_array;
        }
        std::set<std::string> tags = product.get_tags();
        if (!tags.empty()) {
            nlohmann::json tags_array = {};
            for (const auto& tag : tags) {
                tags_array.push_back(tag);
            }
            product_obj["tags"] = tags_array;
        }
        std::vector<Image> images = product.get_images();
        if (!images.empty()) {
            nlohmann::json images_array = {};
            for (const auto& image : images) {
                nlohmann::json image_obj = {};
                image_obj["name"] = image.name;
                image_obj["size"] = image.size;
                image_obj["id"] = image.id;
                bool is_thumbnail = ((images.size() == 1) || (image.id == 0));
                if(is_thumbnail) { 
                    std::cout << image.name << " \033[1;35mwill be used as thumbnail\033[0m\n";
                }
                image_obj["pieces"] = image.pieces;
                image_obj["piece_size"] = image.piece_size;
                
                images_array.push_back(image_obj);
            }
            product_obj["images"] = images_array;
        }
        //------------------------------
        json_object["product"] = product_obj;
        json_object["metadata"] = "listing";
        set_expiration(json_object, json_object["metadata"].get<std::string>());
    }
    
    if(std::holds_alternative<Cart>(obj)) { 
        const Cart& cart = std::get<Cart>(obj);
        json_object["id"] = cart.get_id();
        json_object["owner_id"] = cart.get_owner_id();
        for (const auto& item : cart.contents) {
            nlohmann::json cart_item_obj = {};
            cart_item_obj["product_id"] = item.key;
            cart_item_obj["quantity"] = item.quantity;
            cart_item_obj["seller_id"] = item.seller_id;
            json_object["contents"].push_back(cart_item_obj);
        }
        json_object["metadata"] = "cart";
    }
    
    if(std::holds_alternative<Order>(obj)) {
        const Order& order = std::get<Order>(obj);
        json_object["id"] = order.get_id();
        json_object["created_at"] = order.get_date();
        json_object["status"] = order.get_status_as_string();
        json_object["customer_id"] = order.get_customer_id();
        json_object["subtotal"] = order.get_subtotal();
        json_object["discount"] = order.get_discount();
        json_object["shipping_cost"] = order.get_shipping_cost();
        json_object["total"] = order.get_total();
        auto payment_method = order.get_payment_method();
        json_object["payment_method"] = get_payment_method_as_string(payment_method);
        auto payment_coin = order.get_payment_coin();
        if((payment_method == PaymentMethod::Crypto) &&
            (payment_coin != PaymentCoin::None)) {
            json_object["payment_coin"] = get_payment_coin_as_string(payment_coin);
        }
        json_object["payment_option"] = get_payment_option_as_string(order.get_payment_option());
        auto delivery_option = order.get_delivery_option();
        json_object["delivery_option"] = get_delivery_option_as_string(delivery_option);
        if(delivery_option == DeliveryOption::Shipping) {
            json_object["shipping_option"] = get_shipping_option_as_string(order.shipping_option);
        }
        json_object["notes"] = order.get_notes(); // TODO: encrypt notes
        for(const auto& item : order.get_items()) {
            nlohmann::json order_item_obj = {};
            order_item_obj["listing_key"] = item.key;
            order_item_obj["quantity"] = item.quantity;
            order_item_obj["seller_id"] = item.seller_id;
            json_object["items"].push_back(order_item_obj); // order_items // TODO: encrypt order items
        }
        json_object["metadata"] = "order";
        set_expiration(json_object, json_object["metadata"].get<std::string>());
    }
    
    if(std::holds_alternative<ProductRating>(obj)) {
        const ProductRating& product_rating = std::get<ProductRating>(obj);
        json_object["product_id"] = product_rating.product_id;
        json_object["stars"] = product_rating.stars;
        json_object["rater_id"] = product_rating.rater_id;
        json_object["comments"] = product_rating.comments;
        json_object["signature"] = product_rating.signature;
        json_object["timestamp"] = neroshop::timestamp::get_current_utc_timestamp();
        json_object["metadata"] = "product_rating";
        set_expiration(json_object, json_object["metadata"].get<std::string>());
    }
    
    if(std::holds_alternative<SellerRating>(obj)) {
        const SellerRating& seller_rating = std::get<SellerRating>(obj);
        json_object["seller_id"] = seller_rating.seller_id;
        json_object["score"] = seller_rating.score;
        json_object["rater_id"] = seller_rating.rater_id;
        json_object["comments"] = seller_rating.comments;
        json_object["signature"] = seller_rating.signature;
        json_object["timestamp"] = neroshop::timestamp::get_current_utc_timestamp();
        json_object["metadata"] = "seller_rating";
    }
    
    if(std::holds_alternative<Message>(obj)) {
        const Message& message = std::get<Message>(obj);
        json_object["sender_id"] = message.sender_id;
        json_object["content"] = message.content;
        json_object["recipient_id"] = message.recipient_id;
        json_object["signature"] = message.signature;
        json_object["timestamp"] = neroshop::timestamp::get_current_utc_timestamp();
        json_object["metadata"] = "message";
        set_expiration(json_object, json_object["metadata"].get<std::string>());
    }
    //-------------------------------------------------------------
    // Get the `value`
    std::string value = json_object.dump();
    // Convert the string to a byte string (UTF-8 encoding) in case we ever decide to store values as bytes instead of as json string
    std::vector<uint8_t> value_bytes = nlohmann::json::to_msgpack(json_object);
    // Generate `key` by hashing the value
    std::string key = neroshop::crypto::sha3_256(value);//std::string key = neroshop::crypto::sha3_256(value_bytes);
    //-------------------------------------------------------------
    #ifdef NEROSHOP_DEBUG0
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
    
    // Return key-value pair
    return std::make_pair(key, value);//value_bytes);
}

//-----------------------------------------------------------------------------

std::shared_ptr<neroshop::Object> Serializer::deserialize(const std::pair<std::string, std::string/*std::vector<uint8_t>*/>& data) {//const neroshop::Object& Serializer::deserialize(const std::pair<std::string, std::vector<uint8_t>>& data) {
    // First element of the pair is the key
    std::string key = data.first;
    // Second element of the pair is the serialized value
    ////std::vector<uint8_t> serialized_value = data.second;
    // Deserialize the value
    nlohmann::json value = nlohmann::json::parse(data.second);////nlohmann::json::from_msgpack(serialized_value);
    ////std::string json_string = value.dump();
    // Deserialize the JSON object and create the appropriate variant object
    assert(value["metadata"].is_string());
    std::string metadata = value["metadata"].get<std::string>();
    std::shared_ptr<neroshop::Object> variant_object;
    
    if(metadata == "listing") {
        Listing listing;
        listing.set_id(value["id"].get<std::string>());
        listing.set_seller_id(value["seller_id"].get<std::string>());
        listing.set_quantity(value["quantity"].get<int>());
        listing.set_price(value["price"].get<double>());
        listing.set_currency(value["currency"].get<std::string>());
        listing.set_condition(value["condition"].get<std::string>());
        if (value.contains("location") && value["location"].is_string()) {
            listing.set_location(value["location"].get<std::string>());
        }
        listing.set_date(value["date"].get<std::string>());
        // product
        assert(value["product"].is_object());
        const auto& product_value = value["product"];
        Product product; // initialize the Product object
        product.set_id(/*product_*/value["id"].get<std::string>()); // use listing_id instead
        product.set_name(product_value["name"].get<std::string>());
        product.set_description(product_value["description"].get<std::string>());
        if(product_value.contains("options") && product_value["options"].is_array()) {
            for (const auto& option_key : product_value["options"]) {
                std::string option = option_key.get<std::string>();
                // Just note the option keys if needed, but no storage needed here
            }
        }
        if(product_value.contains("variants") && product_value["variants"].is_array()) {
            for (const auto& variant_json : product_value["variants"]) {
                ProductVariant variant;

                // Deserialize the options map
                if (variant_json.contains("options") && variant_json["options"].is_object()) {
                    for (auto& [key, val] : variant_json["options"].items()) {
                        variant.options[key] = val.get<std::string>();
                    }
                }

                // Deserialize optional fields
                if(variant_json.contains("condition")) {
                    variant.info.condition = variant_json["condition"].get<std::string>();
                }
                if(variant_json.contains("weight")) {
                    variant.info.weight = variant_json["weight"].get<double>();
                }
                if(variant_json.contains("price")) {
                    variant.info.price = variant_json["price"].get<double>();
                }
                if(variant_json.contains("quantity")) {
                    variant.info.quantity = variant_json["quantity"].get<int>();
                }
                if(variant_json.contains("product_code")) {
                    variant.info.product_code = variant_json["product_code"].get<std::string>();
                }
                if(variant_json.contains("image_index")) {
                    variant.info.image_index = variant_json["image_index"].get<int>();
                }

                product.add_variant(variant);
            }
        }
        if (product_value.contains("code")) product.set_code(product_value["code"].get<std::string>());
        product.set_category(product_value["category"].get<std::string>());
        if (product_value.contains("subcategories")) product.set_subcategories(product_value["subcategories"].get<std::vector<std::string>>());
        if (product_value.contains("tags")) product.set_tags(product_value["tags"].get<std::set<std::string>>());
        
        listing.set_product(product); // move the object into the shared_ptr
        variant_object = std::make_shared<Object>(std::move(listing));
    }
    
    if(metadata == "cart") {
        Cart cart;
        cart.set_id(value["id"].get<std::string>());
        cart.set_owner_id(value["owner_id"].get<std::string>());
        assert(value["contents"].is_array());
        std::vector<CartItem> cart_items;
        for (const auto& item : value["contents"]) {
            std::string product_id = item["product_id"].get<std::string>();
            int quantity = item["quantity"].get<int>();
            std::string seller_id = item["seller_id"].get<std::string>();
            cart_items.emplace_back(CartItem{product_id, static_cast<unsigned int>(quantity), seller_id});
        }
        cart.set_contents(cart_items);
        variant_object = std::make_shared<Object>(cart);
    }
    
    if(metadata == "order") {
        Order order;
        order.set_id(value["id"].get<std::string>());
        order.set_date(value["created_at"].get<std::string>());
        order.set_status_by_string(value["status"].get<std::string>());
        order.set_customer_id(value["customer_id"].get<std::string>());
        order.set_subtotal(value["subtotal"].get<double>());
        order.set_discount(value["discount"].get<double>());
        order.set_shipping_cost(value["shipping_cost"].get<double>());
        order.set_total(value["total"].get<double>());
        order.set_payment_option_by_string(value["payment_option"].get<std::string>());
        order.set_payment_coin_by_string(value["payment_coin"].get<std::string>());
        order.set_delivery_option_by_string(value["delivery_option"].get<std::string>());
        order.set_notes(value["notes"].get<std::string>());
        assert(value["items"].is_array()); // items should be an array of objects
        std::vector<OrderItem> order_items;
        for (const auto& item : value["items"]) {
            std::string listing_id = item["listing_key"].get<std::string>();
            int quantity = item["quantity"].get<int>();
            std::string seller_id = item["seller_id"].get<std::string>();
            order_items.emplace_back(OrderItem{listing_id, static_cast<unsigned int>(quantity), seller_id});
        }
        order.set_items(order_items);
        variant_object = std::make_shared<Object>(order);
    }
    
    if(metadata == "product_rating") {
        ProductRating product_rating;
        product_rating.rater_id = value["rater_id"].get<std::string>();
        product_rating.comments = value["comments"].get<std::string>();
        product_rating.signature = value["signature"].get<std::string>();
        product_rating.product_id = value["product_id"].get<std::string>();
        product_rating.stars = value["stars"].get<int>();
        variant_object = std::make_shared<Object>(product_rating);
    }
    
    if(metadata == "seller_rating") {
        SellerRating seller_rating;
        seller_rating.rater_id = value["rater_id"].get<std::string>();
        seller_rating.comments = value["comments"].get<std::string>();
        seller_rating.signature = value["signature"].get<std::string>();
        seller_rating.seller_id = value["seller_id"].get<std::string>();
        seller_rating.score = value["score"].get<int>();
        variant_object = std::make_shared<Object>(seller_rating);
    }
    // Return variant object as shared_ptr
    return variant_object;
}

//-----------------------------------------------------------------------------

std::pair<std::string, std::string> Serializer::serialize(const User& user) {
    nlohmann::json json_object = {};
    const Seller* seller = dynamic_cast<const Seller*>(&user);
    
    std::string display_name = user.get_name();
    if(!display_name.empty()) {
        json_object["display_name"] = display_name;
    }
    std::string user_id = user.get_id();
    json_object["monero_address"] = user_id; // used as identifier
    std::string public_key = user.get_public_key();
    assert(!public_key.empty());
    json_object["public_key"] = public_key;
    Image * avatar = user.get_avatar();
    if(avatar != nullptr) {
        nlohmann::json avatar_obj = {};
        avatar_obj["name"] = avatar->name;
        avatar_obj["size"] = avatar->size;
        avatar_obj["pieces"] = avatar->pieces;
        avatar_obj["piece_size"] = avatar->piece_size;
        json_object["avatar"] = avatar_obj;
    }
    std::string signature = seller->get_wallet()->sign_message(user_id, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    json_object["signature"] = signature;
    json_object["created_at"] = neroshop::timestamp::get_current_utc_timestamp();
    json_object["metadata"] = "user";
    
    // Generate key-value pair
    std::string value = json_object.dump();
    std::string key = neroshop::crypto::sha3_256(value);
    
    // Data verification tests
    #ifdef NEROSHOP_DEBUG0
    auto result = seller->get_wallet()->verify_message(user_id, signature);
    std::cout << "\033[1mverified: " << (result == 1 ? "\033[32mpass" : "\033[91mfail") << "\033[0m\n";
    assert(result == true);
    
    std::vector<uint8_t> value_bytes = nlohmann::json::to_msgpack(json_object);
    nlohmann::json bytes_to_json = nlohmann::json::from_msgpack(value_bytes);
    assert(value == bytes_to_json.dump()); // The value should be the same as the bytes dumped
    #endif
    
    // Return key-value pair
    return std::make_pair(key, value);
}

//-----------------------------------------------------------------------------

std::shared_ptr<neroshop::User> Serializer::deserialize_user(const std::pair<std::string, std::string>& data) {
    // First element of the pair is the key
    std::string key = data.first;
    // Second element of the pair the value
    nlohmann::json value = nlohmann::json::parse(data.second);
    // Deserialize the JSON object and create the appropriate variant object
    std::string metadata = value["metadata"];
    std::shared_ptr<neroshop::User> user_object;
    if(metadata == "user") {
        user_object = std::make_shared<User>();
        user_object->set_id(value["monero_address"].get<std::string>());
        if (value.contains("display_name")) {
            assert(value["display_name"].is_string());
            user_object->set_name(value["display_name"].get<std::string>());
        }
        user_object->set_public_key(value["public_key"].get<std::string>());
        //user_object.set_(value[""].get<std::string>());
    }

    return user_object;
}

//-----------------------------------------------------------------------------
}
