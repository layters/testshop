#include "cart.hpp"

#include "../database/database.hpp"
#include "../tools/logger.hpp"
#include "../protocol/transport/client.hpp"
#include "../tools/uuid.hpp"
#include "../protocol/rpc/protobuf.hpp"

namespace neroshop {
////////////////////
Cart::Cart() : id("") {} // Note: cart can only hold up to 10 unique items. Cart items can only add up to a quantity of 100
////////////////////
unsigned int Cart::max_items(10); 
unsigned int Cart::max_quantity(100);
////////////////////
Cart::~Cart() {
    contents.clear();
#ifdef NEROSHOP_DEBUG
    std::cout << "cart deleted\n";
#endif
}
////////////////////
////////////////////
static nlohmann::json get_listing_object(const std::string& listing_key) {
    neroshop::Client * client = neroshop::Client::get_main_client();
    
    // Get the value of the corresponding key from the DHT
    std::vector<uint8_t> response;
    client->get(listing_key, response);
    // Skip empty and invalid responses
    if(response.empty()) return nlohmann::json();
    #if defined(NEROSHOP_USE_PROTOBUF)
    neroshop::DhtMessage resp_msg;
    if (!resp_msg.ParseFromArray(response.data(), static_cast<int>(response.size()))) {
        return nlohmann::json(); // Parsing error, return empty object
    }
    if (resp_msg.has_error()) {
        log_trace("Received error (get): {}", resp_msg.error().DebugString());
        // Remove obsolete key from local storage
        std::vector<uint8_t> response2;
        client->remove(listing_key, response2);
        neroshop::DhtMessage resp_msg2;
        if (resp_msg2.ParseFromArray(response2.data(), static_cast<int>(response2.size()))) {
            if(resp_msg2.has_response()) log_info("Removed key {}", listing_key);//log_trace("Received response (remove): {}", resp_msg2.response().DebugString());
        }
        return nlohmann::json(); // Key is lost or missing from DHT, return empty object
    } else if(resp_msg.has_response()) {
        log_trace("Received response (get): {}", resp_msg.response().DebugString());
    }
    
    const auto& payload = resp_msg.response().response();
    const auto& data_map = payload.data();
    if (data_map.find("value") != data_map.end()) {
        std::string value = data_map.at("value");
    #endif
        nlohmann::json value_obj = nlohmann::json::parse(value);
        assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
        std::string metadata = value_obj["metadata"].get<std::string>();
        if (metadata != "listing") { std::cerr << "Invalid metadata. \"listing\" expected, got \"" << metadata << "\" instead\n"; return nlohmann::json(); }
        return value_obj;
    }
    return nlohmann::json(); // Return an empty JSON object if "value" is not found or not a string
}

////////////////////
////////////////////
void Cart::load(const std::string& user_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_client_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Set the cart's id
    this->id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });
    // Set the cart's owner id
    this->owner_id = user_id;
    // Lock only for raw handle usage
    {
        ////if (database->is_mutex_enabled()) std::lock_guard<std::mutex> db_lock(database->get_mutex());
        // Prepare (compile) statement
        std::string command = "SELECT listing_key, quantity, seller_id FROM cart_item WHERE cart_id = $1";
        sqlite3_stmt * stmt = nullptr;
        if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            neroshop::log_error("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())));
            return;
        }
        // Check whether the prepared statement returns no data (for example an UPDATE)
        if(sqlite3_column_count(stmt) == 0) {
            neroshop::log_debug("Cart::load(): You have no items your cart");
            return;
        }
        // Bind cart_id to first argument
        if(sqlite3_bind_text(stmt, 1, this->id.c_str(), this->id.length(), SQLITE_STATIC) != SQLITE_OK) {
            neroshop::log_error("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())));
            sqlite3_finalize(stmt);
            return;////database->execute("ROLLBACK;"); return;
        }
        // Get all table values row by row
        while(sqlite3_step(stmt) == SQLITE_ROW) {
            CartItem cart_item; // Create a CartItem object for each row
            for(int i = 0; i < sqlite3_column_count(stmt); i++) {
                std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
                if(column_value == "NULL") { std::cout << "NULL cart_item column skipped"; continue; }
                if(i == 0) cart_item.key = column_value; // listing key
                if(i == 1) cart_item.quantity = std::stoi(column_value); // quantity
                if(i == 2) cart_item.seller_id = column_value; // seller id
            }
            contents.push_back(cart_item);
            ////neroshop::log_info("loaded cart item (id: " + cart_item.key + ", qty: " + std::to_string(cart_item.quantity) + ")");
        }
        // Finalize statement
        sqlite3_finalize(stmt);
    } // Mutex released here
    //----------------------------------
    // Update item quantities based on stock available
    for (auto& cart_item : contents) {
        std::string listing_key = cart_item.key;
        int quantity = cart_item.quantity;
        std::string item_name = listing_key;
        // Get listing object from DHT and use it to check the stock available.
        nlohmann::json listing_obj = get_listing_object(listing_key);
        if (listing_obj.contains("product") && listing_obj["product"].is_object()) {
            nlohmann::json product_obj = listing_obj["product"];
            item_name = product_obj["name"].get<std::string>();
        }
        int stock_available = (!listing_obj.contains("quantity")) ? 0 : listing_obj["quantity"].get<int>();////database->get_integer_params("SELECT quantity FROM listings WHERE listing_key = $1 AND quantity > 0", { listing_key });
        if(stock_available == 0) {
            neroshop::log_error(item_name + " is out of stock");
            // Set item quantity to zero
            database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(0), this->id, listing_key });
            // Remove the item from the user's cart
            database->execute_params("DELETE FROM cart_item WHERE cart_id = $1 AND listing_key = $2", { this->id, listing_key });
            neroshop::log_info(item_name + " (x" + std::to_string(quantity) + ") removed from cart");
            // Remove the item from std::map
            contents.erase(std::remove_if(contents.begin(), contents.end(), [&](const CartItem& item){ return item.key == listing_key; }), contents.end());
            continue; // skip this item since it is no longer in stock, and so we do not store it in the cart            
        }
        // Adjust item quantity to match the stock available
        if(quantity >= stock_available) quantity = stock_available;
        // Update the item's quantity (just to be sure it does not surpass the stock available)
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(quantity), this->id, listing_key });        
        // Update item quantity in memory
        cart_item.quantity = quantity;
        neroshop::log_info("loaded cart item (id: " + listing_key + ", qty: " + std::to_string(quantity) + ")");
    }
    //print_cart();
}
////////////////////
neroshop::CartError Cart::add(const std::string& user_id, const std::string& listing_key, int quantity) {
    neroshop::db::Sqlite3 * database = neroshop::get_client_database();
    if(!database) throw std::runtime_error("database is NULL");
    if(quantity < 1) return CartError::ItemQuantityNotSpecified;
    std::string item_name = listing_key;
    // Get listing object from DHT and use it to check the stock available.
    nlohmann::json listing_obj = get_listing_object(listing_key);
    if (listing_obj.contains("product") && listing_obj["product"].is_object()) {
        nlohmann::json product_obj = listing_obj["product"];
        item_name = product_obj["name"].get<std::string>();
    }    
    // Make sure the user adding the item in the cart isn't the seller itself ... lol
    std::string seller_id = "";
    if(listing_obj.contains("seller_id")) {
        seller_id = listing_obj["seller_id"].get<std::string>();
        if(seller_id == user_id) {
            neroshop::log_error("You can't add your own listings to your cart lol"); 
            return CartError::SellerAddOwnItem;
        }
    }
    // Get cart uuid
    std::string cart_id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });//std::cout << "cart_uuid: " << cart_id << std::endl;
    // Create a cart if cart does not exist in database
    if (cart_id.empty()) {
        cart_id = uuid::generate(); // make a new UUID for this user's cart
        database->execute_params("INSERT INTO cart (uuid, user_id) VALUES (?1, ?2)", { cart_id, user_id });
        log_debug("Cart {} created", cart_id);
    }
    // Cart is full - exit function
    if(is_full()) { 
        neroshop::log_error("Cart is full"); 
        return CartError::Full;
    }
    // Out of stock - exit function
    // TODO: get listing quantity from DHT
    int stock_available = (!listing_obj.contains("quantity")) ? 0 : listing_obj["quantity"].get<int>();////database->get_integer_params("SELECT quantity FROM listings WHERE listing_key = $1 AND quantity > 0", { listing_key });
    if(stock_available == 0) {
        neroshop::log_error(item_name + " is out of stock"); 
        return CartError::ItemOutOfStock;
    }
    //-------------------------------
    // find the index of the tuple containing the specified listing_key
    std::size_t listing_index = get_listing_index(listing_key);
    //-------------------------------
    // Make sure user-specified quantity does not exceed amount in stock
    int item_qty = database->get_integer_params("SELECT quantity FROM cart_item WHERE cart_id = $1 AND listing_key = $2", { cart_id, listing_key });
    // If item quantity surpasses the amount in stock, set item quantity to the amount in stock
    if((item_qty + quantity) > stock_available) {
        neroshop::log_error("Only " + std::to_string(stock_available) + " " + item_name + "s left in stock");
        int remainder = (stock_available - item_qty);
        if(remainder == 0) return CartError::ItemQuantitySurpassed; // If zero then there's no more that you can add to the cart
        quantity = item_qty + remainder; // Increase item quantity by whatever is left of the stock available
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(quantity), cart_id, listing_key });
        neroshop::log_info(std::string("Already in cart: ") + item_name + " +" + std::to_string(remainder) + " (" + std::to_string(quantity) + ")");
        contents[listing_index].quantity = quantity; print_cart(); 
        return CartError::Ok; // exit function since item_qty has surpassed the amount in stock
    }
    //-------------------------------
    // If cart_qty added with the user-specified quantity exceeds max_quantity (cart does not have to be full)
    // Adjust the quantity so it can fit into the cart
    // Example: 100-10=90 (max_quantity-cart_qty=quantity)(you need at least a quantity of 90 to fit everything into the cart)
    int cart_qty = database->get_integer_params("SELECT SUM(quantity) FROM cart_item WHERE cart_id = $1", { cart_id });
    if((cart_qty + quantity) > max_quantity) quantity = max_quantity - cart_qty;////neroshop::log_error(std::string("\033[0;33m") + std::string("Cart is full (max_quantity (") + std::to_string(max_quantity) + ") has been reached)\033[0m"); }
    // If item is already in the cart, just update the quantity and exit the function
    if(in_cart(listing_key) && quantity > 0) {
        int new_quantity = item_qty + quantity;
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(new_quantity), cart_id, listing_key });
        neroshop::log_info(std::string("Already in cart: ") + item_name + " +" + std::to_string(quantity) + " (" + std::to_string(new_quantity) + ")");
        contents[listing_index].quantity = new_quantity; print_cart(); 
        return CartError::Ok;
    }
    // Add item to cart (cart_item)
    database->execute_params("INSERT INTO cart_item (cart_id, listing_key, quantity, seller_id) "
                             "VALUES ($1, $2, $3, $4)", 
    { cart_id, listing_key, std::to_string(quantity), seller_id });
    contents.emplace_back(CartItem{listing_key, static_cast<unsigned int>(quantity), seller_id}); // Save in memory as well
    neroshop::log_info(item_name + " (" + std::to_string(quantity) + ") added to cart");
    print_cart();
    return CartError::Ok;
}
////////////////////
void Cart::remove(const std::string& user_id, const std::string& listing_key, int quantity) {
    neroshop::db::Sqlite3 * database = neroshop::get_client_database();
    if(!database) throw std::runtime_error("database is NULL");
    
}
////////////////////
void Cart::empty() {
    if(id.empty()) { neroshop::log_error("No cart found"); return; }
    neroshop::db::Sqlite3 * database = neroshop::get_client_database();
    if(!database) throw std::runtime_error("database is NULL");
    database->execute_params("DELETE FROM cart_item WHERE cart_id = $1;", { this->id });
    contents.clear();
}
////////////////////
void Cart::change_quantity(const std::string& user_id, const std::string& listing_key, int quantity) {
}
////////////////////
////////////////////
////////////////////
////////////////////
void Cart::print_cart() {
    for (const auto& item : contents) { 
        std::cout << item.key << " (x" << item.quantity << "), sold by " << item.seller_id << std::endl; 
    }
    std::cout << "cart_qty: " << get_quantity() << std::endl;
    std::cout << "cart_items (rows): " << contents.size() << std::endl;
}
////////////////////
////////////////////
////////////////////
void Cart::set_id(const std::string& id) {
    this->id = id;
}

void Cart::set_owner_id(const std::string& owner_id) {
    this->owner_id = owner_id;
}

void Cart::set_contents(const std::vector<CartItem>& contents) {
    this->contents = contents;
}
////////////////////
////////////////////
////////////////////
std::string Cart::get_id() const {
    return id;
}
////////////////////
std::string Cart::get_owner_id() const {
    return owner_id;
}
////////////////////
////////////////////
int Cart::get_max_items() {
    return max_items;
}
////////////////////
int Cart::get_max_quantity() {
    return max_quantity;
}
////////////////////
int Cart::get_quantity() const {
    int quantity = 0;
    for (const auto& item : contents) {
        quantity += item.quantity;
    }
    return quantity;
}
////////////////////
int Cart::get_contents_count() const {
    return contents.size();
}
////////////////////
std::size_t Cart::get_listing_index(const std::string& listing_key) {
    auto it = std::find_if(contents.begin(), contents.end(), [&listing_key](const CartItem& item) {
        return item.key == listing_key;
    });
    return (it != contents.end()) ? std::distance(contents.begin(), it) : contents.size(); // return index if found, else size of vector
}
////////////////////
////////////////////
////////////////////
////////////////////
bool Cart::is_empty() const {
    return contents.empty();
}
////////////////////
bool Cart::is_full() const {
    int quantity = get_quantity();
    return ((contents.size() >= max_items) || (quantity >= max_quantity)); // either cart.contents has reached its capacity (max_items:10) or all items have a combined quantity of 100 (max_quantity:100)
}
////////////////////
bool Cart::in_cart(const std::string& listing_key) const {
    for (const auto& item : contents) {
        if (item.key == listing_key) {
            return true;
        }
    }
    return false;
}
////////////////////
////////////////////
////////////////////
////////////////////
}
