#include "user.hpp"

#include "../../neroshop_config.hpp"
#include "cart.hpp"
#include "rating.hpp"
#include "../settings.hpp"
#include "../database/database.hpp"
#include "../tools/logger.hpp"
#include "../protocol/p2p/serializer.hpp"
#include "../protocol/transport/client.hpp"
#include "../crypto/rsa.hpp"
#include "../crypto/sha3.hpp"
#include "../tools/base64.hpp"
#include "../tools/timestamp.hpp"
#include "../tools/string.hpp"
#include "../wallet/wallet.hpp"

#include <fstream>
#include <regex>

namespace neroshop {
////////////////////
User::User() : wallet(nullptr), id(""), logged(false), account_type(UserAccountType::Guest), cart(nullptr), order_list({}), favorites({}) {
    cart = std::unique_ptr<Cart>(new Cart());
}
////////////////////
User::~User()
{
    // clear private key
    private_key.clear();
    // destroy cart
    if(cart.get()) cart.reset();
    // destroy wallet
    if(wallet.get()) wallet.reset();
    // clear orders
    order_list.clear(); // this should reset (delete) all orders
    // clear favorites
    favorites.clear(); // this should reset (delete) all favorites
#ifdef NEROSHOP_DEBUG
    std::cout << "user deleted\n";
#endif    
}
////////////////////
////////////////////
////////////////////
void User::rate_seller(const std::string& seller_id, int score, const std::string& comments, const std::string& signature) { // perfected 99.9%!!
    if(seller_id.empty()) return;
    
    // score must be between 0 and 1
    if(score >= 1) score = 1;
    if(score <= 0) score = 0;
    
    // Prevent seller from rating him/herself
    if(seller_id == this->id) {
        std::cerr << "\033[91mYou cannot rate yourself\033[0m\n";
        return; // exit function
    }
    
    // TODO: Buyers should only be allowed to rate sellers they have purchased from
    
    Client * client = Client::get_main_client();
    //----------------------------------
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = ?1 AND content = 'seller_rating'";
    db::Sqlite3 * database = neroshop::get_database();
    sqlite3_stmt * stmt = nullptr;

    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::log_error("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())));
        return;
    }
    
    if(sqlite3_bind_text(stmt, 1, seller_id.c_str(), seller_id.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::log_error("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())));
        sqlite3_finalize(stmt);
        return;
    }
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        for(int i = 0; i < sqlite3_column_count(stmt); i++) { 
            const char* column_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_text  << " (" << i << ")" << std::endl;
            std::string key = (column_text != nullptr) ? column_text : "";
            if(key.empty()) { continue; } // Skip invalid columns//std::cout << key << "\n";
            
            // Get the value of the corresponding key from the DHT
            std::string response;
            client->get(key, response); // TODO: error handling
            std::cout << "Received response (get): " << response << "\n";
            // Parse the response
            nlohmann::json json = nlohmann::json::parse(response);
            if(json.contains("error")) {
                std::string response2;
                client->remove(key, response2);
                std::cout << "Received response (remove): " << response2 << "\n";
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                std::string metadata = value_obj["metadata"].get<std::string>();
                if (metadata != "seller_rating") { std::cerr << "Invalid metadata. \"seller_rating\" expected, got \"" << metadata << "\" instead\n"; continue; }
                // Check if the rater (you) has not already rated this seller
                std::string rater_id = value_obj["rater_id"].get<std::string>();
                if(rater_id == this->id) {
                    std::cerr << "\033[1;33mYou have previously rated this seller\033[0m\n";
                    // Self-verify the signature
                    std::string old_comments = value_obj["comments"].get<std::string>();
                    std::string old_signature = value_obj["signature"].get<std::string>();
                    bool self_verified = wallet->verify_message(old_comments, old_signature);
                    if(!self_verified) { neroshop::log_error("Data verification failed."); return; }
                    // Modify/Update the seller rating and re-signed to reflect the modification
                    value_obj["comments"] = comments;
                    value_obj["score"] = score;
                    assert(old_signature != signature && "Signature is outdated");
                    value_obj["signature"] = signature;
                    value_obj["last_updated"] = neroshop::timestamp::get_current_utc_timestamp();
                    // Send set request containing the updated value with the same key as before
                    std::string modified_value = value_obj.dump();
                    std::string response;
                    client->set(key, modified_value, response); // key MUST remain unchanged!!
                    std::cout << "Received response (set): " << response << "\n";
                    return;
                }
            }
        }
    }

    sqlite3_finalize(stmt);
    //----------------------------------
    SellerRating seller_rating = { this->id, comments, signature, seller_id, static_cast<unsigned int>(score) };
    
    auto data = Serializer::serialize(seller_rating);
    std::string key = data.first;
    std::string value = data.second;//std::cout << "key: " << data.first << "\nvalue: " << data.second << "\n";
    
    // Send put request to neighboring nodes (and your node too JIC)
    std::string response;
    client->put(key, value, response);
    std::cout << "Received response: " << response << "\n";
} 
////////////////////
////////////////////
void User::rate_item(const std::string& product_id, int stars, const std::string& comments, const std::string& signature) { // perfected 99%!!!
    if(product_id.empty()) return;
    
    // star ratings must be between 1 and 5
    if(stars >= 5) stars = 5;
    if(stars <= 1) stars = 1;
    
    // TODO: Check if the rater has previously purchased this product to be able to rate it
    // TODO: If corresponding listing has an expiration date, add an expiration date to product rating too
    
    Client * client = Client::get_main_client();
    //----------------------------------
    std::string command = "SELECT DISTINCT key FROM mappings WHERE search_term = ?1 AND content = 'product_rating'";
    db::Sqlite3 * database = neroshop::get_database(); 
    sqlite3_stmt * stmt = nullptr;

    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::log_error("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())));
        return;
    }
    
    if(sqlite3_bind_text(stmt, 1, product_id.c_str(), product_id.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::log_error("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())));
        sqlite3_finalize(stmt);
        return;
    }
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        for(int i = 0; i < sqlite3_column_count(stmt); i++) { 
            const char* column_text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));//std::cout << column_text  << " (" << i << ")" << std::endl;
            std::string key = (column_text != nullptr) ? column_text : "";
            if(key.empty()) { continue; } // Skip invalid columns//std::cout << key << "\n";
            
            // Get the value of the corresponding key from the DHT
            std::string response;
            client->get(key, response); // TODO: error handling
            std::cout << "Received response (get): " << response << "\n";
            // Parse the response
            nlohmann::json json = nlohmann::json::parse(response);
            if(json.contains("error")) {
                std::string response2;
                client->remove(key, response2);
                std::cout << "Received response (remove): " << response2 << "\n";
                continue; // Key is lost or missing from DHT, skip to next iteration
            }
            
            const auto& response_obj = json["response"];
            assert(response_obj.is_object());
            if (response_obj.contains("value") && response_obj["value"].is_string()) {
                const auto& value = response_obj["value"].get<std::string>();
                nlohmann::json value_obj = nlohmann::json::parse(value);
                assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
                std::string metadata = value_obj["metadata"].get<std::string>();
                if (metadata != "product_rating") { std::cerr << "Invalid metadata. \"product_rating\" expected, got \"" << metadata << "\" instead\n"; continue; }
                // Check if the rater (you) has not already rated this product
                std::string rater_id = value_obj["rater_id"].get<std::string>();
                if(rater_id == this->id) {
                    std::cerr << "\033[91mYou have already rated this product or service\033[0m\n";
                    // Self-verify the signature
                    std::string old_comments = value_obj["comments"].get<std::string>();
                    std::string old_signature = value_obj["signature"].get<std::string>();
                    bool self_verified = wallet->verify_message(old_comments, old_signature);
                    if(!self_verified) { neroshop::log_error("Data verification failed."); return; }
                    // Modify/Update the product rating and re-signed to reflect the modification
                    value_obj["comments"] = comments;
                    value_obj["stars"] = stars;
                    assert(old_signature != signature && "Signature is outdated");
                    value_obj["signature"] = signature;
                    value_obj["last_updated"] = neroshop::timestamp::get_current_utc_timestamp();
                    // Send set request containing the updated value with the same key as before
                    std::string modified_value = value_obj.dump();
                    std::string response;
                    client->set(key, modified_value, response); // key MUST remain unchanged!!
                    std::cout << "Received response (set): " << response << "\n";
                    return;
                }
            }
        }
    }

    sqlite3_finalize(stmt);
    //----------------------------------
    ProductRating product_rating = { this->id, comments, signature, product_id, static_cast<unsigned int>(stars) };
    
    auto data = Serializer::serialize(product_rating);
    std::string key = data.first;
    std::string value = data.second;//std::cout << "key: " << data.first << "\nvalue: " << data.second << "\n";
    
    // Send put request to neighboring nodes (and your node too JIC)
    std::string response;
    client->put(key, value, response);
    std::cout << "Received response: " << response << "\n";
} 
////////////////////
////////////////////
// account-related stuff here
////////////////////
void User::delete_account() {

}
////////////////////
////////////////////
////////////////////
// cart-related stuff here
////////////////////
int User::add_to_cart(const std::string& listing_key, int quantity) {
    neroshop::CartError error = cart->add(this->id, listing_key, quantity);
    return static_cast<int>(error);
}
////////////////////
void User::remove_from_cart(const std::string& listing_key, int quantity) {
    cart->remove(this->id, listing_key, quantity);
}
////////////////////
void User::clear_cart() {
    cart->empty();
}
////////////////////
////////////////////
////////////////////
// order-related stuff here
////////////////////
void User::create_order(const std::string& shipping_address) {//const {
    // name(first, last), address1(street, p.o box, company name, etc.), address2(apt number, suite, unit, building, floor, etc.) city, zip/postal_code, state/province/region country, optional(phone, email)
    try {
        std::shared_ptr<neroshop::Order> order(std::make_shared<neroshop::Order>());//(new neroshop::Order());
        order->create_order(*cart.get(), shipping_address); // we are using crypto, not debit/credit cards so no billing address is needed
        order_list.push_back(order); // whether an order fails or succeeds, store it regardless
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
}
////////////////////
// put this in Seller::on_login
// orders are never deleted, their statuses just change: rejected, failure, delivered, etc.
void User::load_orders() {

}
////////////////////
////////////////////
////////////////////
// favorite-or-wishlist-related stuff
////////////////////
void User::add_to_favorites(const std::string& listing_key) {
    db::Sqlite3 * database = neroshop::get_client_database();

    // check if item is already in favorites so that we do not add the same item more than once
    bool favorited = database->get_integer_params("SELECT EXISTS(SELECT listing_key FROM favorites WHERE listing_key = ?1 AND user_id = ?2)", { listing_key, this->id });
    if(favorited) { neroshop::log_warn("\"" + listing_key + "\" is already in your favorites"); return; }
    // add item to favorites
    int rescode = database->execute_params("INSERT INTO favorites (user_id, listing_key) VALUES (?1, ?2);", { this->id, listing_key });
    if(rescode != SQLITE_OK) {
        neroshop::log_error("failed to add item to favorites");
        return;
    }
    // store in memory as well
    favorites.push_back(listing_key);
    if(std::find(favorites.begin(), favorites.end(), listing_key) != favorites.end()) {
        neroshop::log_info("\"" + listing_key + "\" has been added to your favorites");
    }
}
////////////////////
void User::remove_from_favorites(const std::string& listing_key) {
    db::Sqlite3 * database = neroshop::get_client_database();
    
    // check if item has already been removed from favorites so that we don't have to remove it more than once
    bool favorited = database->get_integer_params("SELECT EXISTS(SELECT listing_key FROM favorites WHERE listing_key = ?1 AND user_id = ?2)", { listing_key, this->id });
    if(!favorited) {
        auto it = std::find(favorites.begin(), favorites.end(), listing_key);
        if (it != favorites.end()) { favorites.erase(it); } // remove from vector if found in-memory, but not found in database
        neroshop::log_warn("\"" + listing_key + "\" is not in your favorites");
        return;
    }
    // remove item from favorites (database)
    int rescode = database->execute_params("DELETE FROM favorites WHERE listing_key = ?1 AND user_id = ?2;", { listing_key, this->id });
    if (rescode != SQLITE_OK) {
        neroshop::log_error("failed to remove item from favorites");
        return;
    }
    // remove from vector as well
    auto it = std::find(favorites.begin(), favorites.end(), listing_key);
    if (it != favorites.end()) {
        favorites.erase(it);
        if(std::find(favorites.begin(), favorites.end(), listing_key) == favorites.end()) neroshop::log_info("\"" + listing_key + "\" has been removed from your favorites"); // confirm that item has been removed from favorites
    }
}
////////////////////
void User::clear_favorites() {
    db::Sqlite3 * database = neroshop::get_client_database();
    
    // first check if favorites (database table) is empty
    int favorites_count = database->get_integer_params("SELECT COUNT(*) FROM favorites WHERE user_id = ?1", { this->id });
    if(favorites_count <= 0) return; // table is empty so that means there is nothing to delete, exit function
    // clear all items from favorites
    int rescode = database->execute_params("DELETE FROM favorites WHERE user_id = ?1", { this->id });
    if (rescode != SQLITE_OK) {
        neroshop::log_error("failed to clear favorites");
        return;
    }
    // clear favorites from vector as well
    favorites.clear();
    if(favorites.empty()) neroshop::log_info("your favorites have been cleared"); // confirm that favorites has been cleared
}
////////////////////
void User::load_favorites() {
    favorites.clear();    
    db::Sqlite3 * database = neroshop::get_client_database();
    std::string command = "SELECT DISTINCT listing_key FROM favorites WHERE user_id = ?1;";
    sqlite3_stmt * stmt = nullptr;
    // Prepare (compile) statement
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::log_error("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())));
        return;
    }
    // Bind this->id to first argument
    if(sqlite3_bind_text(stmt, 1, this->id.c_str(), this->id.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::log_error("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())));
        sqlite3_finalize(stmt);
        return;
    }    
    
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string listing_key = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            if(listing_key == "NULL") continue; // Skip invalid columns            
            favorites.push_back(listing_key); // store favorited listings for later use
            if(std::find(favorites.begin(), favorites.end(), listing_key) != favorites.end()) {
                neroshop::log_debug("Favorited item (" + listing_key + ") has been loaded");
            }
        }
    }
    
    sqlite3_finalize(stmt);
}
////////////////////
////////////////////
////////////////////
// avatar-related stuff here
////////////////////
void User::upload_avatar(const std::string& filename) {
    // Get image size
    std::ifstream image_file(filename, std::ios::binary);
    if(!image_file.good()) {
        std::cout << "failed to load " << filename << std::endl; 
        return;
    }
    image_file.seekg(0, std::ios::end);
    size_t size = static_cast<int>(image_file.tellg()); // in bytes
    image_file.close();
    
    // Construct image object
    Image image;
    std::string image_name = filename.substr(filename.find_last_of("\\/") + 1);
    std::string image_name_without_ext = image_name.substr(0, image_name.find_last_of(".")); // remove extension
    std::string image_ext = filename.substr(filename.find_last_of(".") + 1);
    image.name = neroshop::crypto::sha3_256(image_name_without_ext) + "." + image_ext;
    image.size = size;
    image.source = filename;
    
    avatar = std::make_unique<Image>(std::move(image));
}
////////////////////
void User::delete_avatar() {
 
}
////////////////////
////////////////////
void User::send_message(const std::string& recipient_id, const std::string& content, const std::string& public_key) {
    if(recipient_id == this->id) {
        neroshop::log_error("You cannot message yourself");
        return;
    }
    
    // Construct message
    Message message;
    //----------------------------------------------------
    int padding_overhead = 42; // only an estimate - probably accurate since I tested it once
    int MAX_DATA_LENGTH_BYTES = (NEROSHOP_RSA_DEFAULT_BITS / 8) - padding_overhead; // RSA-OAEP padding reduces the max data size by 41 to 66 bytes :(
    // Encrypt sender
    std::string sender_encrypted = neroshop::crypto::rsa_public_encrypt(public_key, this->id);//std::cout << "sender (encrypted): " << sender_encrypted << std::endl;
    
    // Convert to base64 (for transmission)
    std::string sender_encoded = neroshop::base64_encode(sender_encrypted);
    message.sender_id = sender_encoded;
    
    #ifdef NEROSHOP_DEBUG0
    std::cout << "sender (base64 encoded): " << sender_encoded << std::endl;
    std::string sender_decoded = neroshop::base64_decode(sender_encoded);
    std::cout << "sender (base64 decoded): " << sender_decoded << std::endl << std::endl;
    #endif
    //----------------------------------------------------
    // Encrypt message
    std::string message_encrypted = neroshop::crypto::rsa_public_encrypt(public_key, content);//std::cout << "message (encrypted): " << message_encrypted << std::endl;
    if(message_encrypted.empty()) {
        neroshop::log_error("Error encrypting message");
        return;
    }
    
    // Convert to base64 (for transmission)
    std::string message_encoded = neroshop::base64_encode(message_encrypted);
    message.content = message_encoded;

    #ifdef NEROSHOP_DEBUG0
    std::cout << "message (base64 encoded): " << message_encoded << std::endl;
    std::string message_decoded = neroshop::base64_decode(message_encoded);
    std::cout << "message (base64 decoded): " << message_decoded << std::endl << std::endl;
    #endif
    //----------------------------------------------------
    message.recipient_id = recipient_id;
    message.signature = wallet->sign_message(content, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
    
    auto data = Serializer::serialize(message);
    std::string key = data.first;
    std::string value = data.second;//std::cout << "key: " << key << "\nvalue: " << value << "\n";
    
    // Send put request to neighboring nodes (and your node too JIC)
    Client * client = Client::get_main_client();
    std::string response;
    client->put(key, value, response);
    #ifdef NEROSHOP_DEBUG
    std::cout << "Received response: " << response << "\n";
    #endif
}
////////////////////
std::pair<std::string, std::string> User::decrypt_message(const std::string& content_encoded, const std::string& sender_encoded) {
    // Decode encoded sender
    std::string sender_decoded = neroshop::base64_decode(sender_encoded);
    
    // Decrypt sender using your own private keys
    std::string sender = neroshop::crypto::rsa_private_decrypt(this->private_key, sender_decoded);
    
    #ifdef NEROSHOP_DEBUG0
    std::cout << "sender (base64 decoded): " << sender_decoded << std::endl;
    std::cout << "sender (decrypted): " << sender << std::endl << std::endl; 
    #endif
    //----------------------------------------------------
    // Decode encoded message
    std::string message_decoded = neroshop::base64_decode(content_encoded);
    
    // Decrypt message using your own private keys
    std::string message = neroshop::crypto::rsa_private_decrypt(this->private_key, message_decoded);
    
    #ifdef NEROSHOP_DEBUG0
    std::cout << "message (base64 decoded): " << message_decoded << std::endl;
    std::cout << "message (decrypted): " << message << std::endl << std::endl;
    #endif
    //----------------------------------------------------
    return std::make_pair(message, sender);
}
////////////////////
////////////////////
void User::set_public_key(const std::string& public_key) {
    // TODO: validate public key before setting it
    this->public_key = public_key;
}
////////////////////
void User::set_private_key(const std::string& private_key) {
    this->private_key = private_key;
}
////////////////////
void User::set_wallet(const neroshop::Wallet& wallet) {
    std::unique_ptr<neroshop::Wallet> user_wallet(&const_cast<neroshop::Wallet&>(wallet));
    this->wallet = std::move(user_wallet);
}
////////////////////
////////////////////
////////////////////
////////////////////
//void User::set_id(unsigned int id) {
//    this->id = id;
//}
////////////////////
void User::set_id(const std::string& id) {
    this->id = id;
}
////////////////////
void User::set_name(const std::string& name) {
    this->name = name;
}
////////////////////
void User::set_account_type(UserAccountType account_type) {
    this->account_type = account_type;
}
////////////////////
void User::set_logged(bool logged) { // protected function, so only derived classes can use this
    this->logged = logged;
    if(!logged) logout(); // call on_logout() (callback)
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
//unsigned int User::get_id() const {
//    return id;
//}
////////////////////
std::string User::get_id() const {
    return id;
}
////////////////////
std::string User::get_name() const {
    return name;
}
////////////////////
UserAccountType User::get_account_type() const {
    return account_type;
}
////////////////////
std::string User::get_account_type_string() const {
    switch(this->account_type) {
        case UserAccountType::Guest: return "Guest"; break;
        case UserAccountType::Buyer: return "Buyer"; break;
        case UserAccountType::Seller: return "Seller"; break;
        default: return ""; break;
    }
}
////////////////////
neroshop::Image * User::get_avatar() const {
    return avatar.get();
}

std::string User::get_public_key() const {
    return public_key;
}

std::string User::get_private_key() const {
    return private_key;
}
////////////////////
neroshop::Wallet * User::get_wallet() const {
    return wallet.get();
}
////////////////////
////////////////////
neroshop::Cart * User::get_cart() const {
    return cart.get();
}
////////////////////
////////////////////
neroshop::Order * User::get_order(unsigned int index) const {
    if(index > (order_list.size() - 1)) throw std::out_of_range("User::get_order(): attempt to access invalid index");
    return order_list[index].get();
}
////////////////////
unsigned int User::get_order_count() const {
    return order_list.size();
}
////////////////////
std::vector<neroshop::Order *> User::get_order_list() const {
    std::vector<neroshop::Order *> orders = {};
    for(const auto & order : order_list) {//for(int o = 0; o < order_list.size(); o++) {
        orders.push_back(order.get());//(order_list[o].get());
    }
    return orders;
}
////////////////////
////////////////////
std::string User::get_favorite(unsigned int index) const {
    if(index > (favorites.size() - 1)) throw std::out_of_range("User::get_favorites(): attempt to access invalid index");
    return favorites[index];
}
////////////////////
unsigned int User::get_favorites_count() const {
    return favorites.size();
}
////////////////////
std::vector<std::string> User::get_favorites() const {
    return favorites;
}
////////////////////
////////////////////
int User::get_account_age(const std::string& user_id) {
    db::Sqlite3 * database = neroshop::get_database();
    std::string key = database->get_text_params("SELECT DISTINCT key FROM mappings WHERE search_term = ?1 AND content = 'user' LIMIT 1", { user_id });
    if(key.empty()) { return -1; }
    
    Client * client = Client::get_main_client();
    std::string response;
    client->get(key, response);
    
    nlohmann::json json = nlohmann::json::parse(response, nullptr, false);
    if(json.is_discarded()) { return -1; }
    if(!json.is_object()) { return -1; }
    if(!json.contains("response")) { return -1; }
    if(json.contains("error")) {
        std::string response2;
        client->remove(key, response2);
        std::cout << "Received response (remove): " << response2 << "\n";
        return -1;
    }
    
    std::string iso8601;
    const auto& response_obj = json["response"];
    if(!response_obj.is_object()) { return -1; }
    if(!response_obj.contains("value")) { return -1; }
    if(!response_obj["value"].is_string()) { return -1; }
    const auto& value = response_obj["value"].get<std::string>();
    nlohmann::json value_obj = nlohmann::json::parse(value, nullptr, false);
    if(value_obj.is_discarded()) { return -1; }
    if(!value_obj.is_object()) { return -1; }
    if(!value_obj.contains("metadata")) { return -1; }
    if(!value_obj["metadata"].is_string()) { return -1; }
    std::string metadata = value_obj["metadata"].get<std::string>();
    if (metadata != "user") { std::cerr << "Invalid metadata. \"user\" expected, got \"" << metadata << "\" instead\n"; return -1; }
    if(!value_obj.contains("created_at")) { return -1; }
    if(!value_obj["created_at"].is_string()) { return -1; }
    iso8601 = value_obj["created_at"].get<std::string>();
    //------------------------------------------------------
    std::tm t = {};
    std::istringstream ss(iso8601);
    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");
    
    // Handling optional fractional seconds and timezone offset
    if (ss.fail()) {
        ss.clear();
        ss.str(iso8601);
        ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S.%f");
    }
    
    if (ss.fail()) {
        ss.clear();
        ss.str(iso8601);
        ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
    }

    if (ss.fail()) {
        throw std::runtime_error("Failed to parse ISO 8601 timestamp");
    }
    //------------------------------------------------------
    auto account_creation_time = std::chrono::system_clock::from_time_t(std::mktime(&t));
    auto now = std::chrono::system_clock::now();

    auto duration = now - account_creation_time;

    using std_chrono_days = std::chrono::duration<int, std::ratio<86400>>;
    auto days = std::chrono::duration_cast<std_chrono_days>(duration).count();
    auto years = days / 365;
    days %= 365;
    auto months = days / 30;
    days %= 30;

    return days;
}
////////////////////
int User::get_account_age() const {
    return get_account_age(this->id);
}
////////////////////
////////////////////
////////////////////
bool User::is_guest() const {
    if(is_logged()) return false;
    return true; // guests (buyers) are not required to register // guests are buyers by default, except their data is not stored
}
////////////////////
bool User::is_buyer() const// buyer and guests are not required to register, only sellers
{
    return true;
}
////////////////////
bool User::is_seller() const
{
    return true;
}
////////////////////
bool User::is_online() const // a user is not created until they are logged so this function can only be called when a user is logged // guests can also use this function so its a bad idea to check if user is logged
{
    return Client::get_main_client()->is_connected();// && is_logged()); // user must be both connected to the network and logged in
}
////////////////////
bool User::is_registered() const {
    return true;
}
////////////////////
bool User::is_registered(const std::string& name) { // no need to login to prove user is registered, just need to check the db
    return true; 
}
////////////////////
bool User::is_logged() const
{
    return logged;
}
////////////////////
bool User::has_email() const {
    return false;    
}
////////////////////
bool User::has_avatar() const {
    return false;
}
////////////////////
////////////////////
bool User::has_purchased(const std::string& product_id) { // for registered users only//if(!is_logged()) { neroshop::log_warn("You are not logged in"); return false; }
    return false;
}
////////////////////
bool User::has_favorited(const std::string& listing_key) {
    // since we loaded the favorites into memory when the app launched, we should be able to access the pre-loaded favorites and any newly added favorites in the current session without performing any database queries/operations
    for(const auto & favorite : favorites) {
        // if any favorites items' ids matches "listing_key" then return true
        if(favorite == listing_key) return true;
    }
    return false;////return (std::find(favorites.begin(), favorites.end(), listing_key) != favorites.end()); // this is good for when storing favorites as integers (product_ids)
}
////////////////////
bool User::has_wallet() const {
    if(!wallet.get()) return false; // wallet is nullptr
    if(!wallet->get_monero_wallet()) return false; // wallet not opened
    return true;
}
////////////////////
bool User::has_wallet_synced() const {
    if(!has_wallet()) return false; // wallet is either nullptr or not opened
    if(!wallet->get_monero_wallet()->is_synced()) return false; // wallet not synced to daemon
    return true;
}
////////////////////
////////////////////
////////////////////
// callbacks
////////////////////
//User * User::on_login(const std::string& username) {return nullptr;} // this function does nothing
////////////////////
void User::on_order_received() {} // for sellers to implement // this function does nothing
////////////////////
void User::logout() {
    //edit: guests can definitely logout too//if(is_guest()) return; // guests don't have an account so therefore they cannot logout
    // do something when logged is set to false ...
    // reset private members to their default values
    this->id = ""; // clear id
    this->name.clear(); // clear name
    this->account_type = UserAccountType::Guest; // set account type to the default
    this->logged = false; // make sure user is no longer logged in
    // delete this user
    if(this) delete this;//this = nullptr;//fails
    // disconnect from server
    // print message    
    neroshop::log_info("You have logged out");
}
////////////////////
////////////////////
////////////////////
}
