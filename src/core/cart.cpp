#include "cart.hpp"

#include "database/database.hpp"
#include "tools/logger.hpp"
#include "protocol/transport/client.hpp"

////////////////////
neroshop::Cart::Cart() : id("") {} // Note: cart can only hold up to 10 unique items. Cart items can only add up to a quantity of 100
////////////////////
unsigned int neroshop::Cart::max_items(10); 
unsigned int neroshop::Cart::max_quantity(100);
////////////////////
neroshop::Cart::~Cart() {
    contents.clear(); // this should reset/delete all cart items (if they happen to be dynamic objects)
#ifdef NEROSHOP_DEBUG
    std::cout << "cart deleted\n";
#endif
}
////////////////////
////////////////////
static nlohmann::json get_listing_object(const std::string& listing_key) {
    neroshop::Client * client = neroshop::Client::get_main_client();
    
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Get the value of the corresponding key from the DHT
    std::string response;
    client->get(listing_key, response);
    std::cout << "Received response (get): " << response << "\n";
    // Parse the response
    nlohmann::json json = nlohmann::json::parse(response);
    if(json.contains("error")) {
        std::cout << "get_available_stock: listing key is lost or missing from DHT\n";
        int rescode = database->execute_params("DELETE FROM mappings WHERE key = ?1", { listing_key });
        if(rescode != SQLITE_OK) neroshop::print("sqlite error: DELETE failed", 1);
        return nlohmann::json(); // Key is lost or missing from DHT
    }
    
    const auto& response_obj = json["response"];
    assert(response_obj.is_object());
    if (response_obj.contains("value") && response_obj["value"].is_string()) {
        const auto& value = response_obj["value"].get<std::string>();
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
void neroshop::Cart::load(const std::string& user_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Set the cart's id
    this->id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });
    // Set the cart's owner id
    this->owner_id = user_id;
    // Prepare (compile) statement
    std::string command = "SELECT listing_key, quantity, seller_id FROM cart_item WHERE cart_id = $1";
    sqlite3_stmt * stmt = nullptr;
    if(sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return;
    }
    // Check whether the prepared statement returns no data (for example an UPDATE)
    if(sqlite3_column_count(stmt) == 0) {
        neroshop::print("Cart::load(): You have no items your cart", 2);
        return;
    }
    // Bind cart_id to first argument
    if(sqlite3_bind_text(stmt, 1, this->id.c_str(), this->id.length(), SQLITE_STATIC) != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text (arg: 1): " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(stmt);
        return;////database->execute("ROLLBACK;"); return;
    }
    // Get all table values row by row
    while(sqlite3_step(stmt) == SQLITE_ROW) {
        std::tuple<std::string, int, std::string> cart_item; // Create a tuple object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            if(column_value == "NULL") { std::cout << "NULL cart_item column skipped"; continue; }
            if(i == 0) std::get<0>(cart_item) = column_value; // listing key
            if(i == 1) std::get<1>(cart_item) = std::stoi(column_value); // quantity
            if(i == 2) std::get<2>(cart_item) = column_value; // seller id
        }
        contents.push_back(cart_item);
        ////neroshop::print("loaded cart item (id: " + std::get<0>(cart_item) + ", qty: " + std::to_string(std::get<1>(cart_item)) + ")", 3);
    }
    /////////////////////////////
    // Update item quantities based on stock available
    for (auto& cart_item : contents) {
        std::string listing_key = std::get<0>(cart_item);
        int quantity = std::get<1>(cart_item);
        std::string item_name = listing_key;
        // Get listing object from DHT and use it to check the stock available.
        nlohmann::json listing_obj = get_listing_object(listing_key);
        if (listing_obj.contains("product") && listing_obj["product"].is_object()) {
            nlohmann::json product_obj = listing_obj["product"];
            item_name = product_obj["name"].get<std::string>();
        }
        int stock_available = (!listing_obj.contains("quantity")) ? 0 : listing_obj["quantity"].get<int>();////database->get_integer_params("SELECT quantity FROM listings WHERE listing_key = $1 AND quantity > 0", { listing_key });
        if(stock_available == 0) {
            neroshop::print(item_name + " is out of stock", 1);
            // Set item quantity to zero
            database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(0), this->id, listing_key });
            // Remove the item from the user's cart
            database->execute_params("DELETE FROM cart_item WHERE cart_id = $1 AND listing_key = $2", { this->id, listing_key });
            neroshop::print(item_name + " (x" + std::to_string(quantity) + ") removed from cart", 1);
            // Remove the item from std::map
            contents.erase(std::remove_if(contents.begin(), contents.end(), [&](auto const& item){ return std::get<0>(item) == listing_key; }), contents.end());
            continue; // skip this item since it is no longer in stock, and so we do not store it in the cart            
        }
        // Adjust item quantity to match the stock available
        if(quantity >= stock_available) quantity = stock_available;
        // Update the item's quantity (just to be sure it does not surpass the stock available)
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(quantity), this->id, listing_key });        
        // Update item quantity in memory
        std::get<1>(cart_item) = quantity;
        neroshop::print("loaded cart item (id: " + listing_key + ", qty: " + std::to_string(quantity) + ")", 3);
    }
    //print_cart();
}
////////////////////
void neroshop::Cart::add(const std::string& user_id, const std::string& listing_key, int quantity) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    if(quantity < 1) return;
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
            neroshop::print("Bruh WTF. You can't add your own listings to your cart lol", 1); return;
        }
    }
    // Get cart uuid
    std::string cart_id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });//std::cout << "cart_uuid: " << cart_id << std::endl;
    // Cart is full - exit function
    if(is_full()) { neroshop::print("Cart is full", 1); return; }
    // Out of stock - exit function
    // TODO: get listing quantity from DHT
    int stock_available = (!listing_obj.contains("quantity")) ? 0 : listing_obj["quantity"].get<int>();////database->get_integer_params("SELECT quantity FROM listings WHERE listing_key = $1 AND quantity > 0", { listing_key });
    if(stock_available == 0) {
        neroshop::print(item_name + " is out of stock", 1); return;
    }
    //-------------------------------
    // find the index of the tuple containing the specified listing_key
    std::size_t listing_index = get_listing_index(listing_key);
    //-------------------------------
    // Make sure user-specified quantity does not exceed amount in stock
    int item_qty = database->get_integer_params("SELECT quantity FROM cart_item WHERE cart_id = $1 AND listing_key = $2", { cart_id, listing_key });
    // If item quantity surpasses the amount in stock, set item quantity to the amount in stock
    if((item_qty + quantity) > stock_available) {
        neroshop::print("Only " + std::to_string(stock_available) + " " + item_name + "s left in stock", 1);
        int remainder = (stock_available - item_qty);
        if(remainder == 0) return; // If zero then there's no more that you can add to the cart
        quantity = item_qty + remainder; // Increase item quantity by whatever is left of the stock available
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(quantity), cart_id, listing_key });
        neroshop::print(std::string("Already in cart: ") + item_name + " +" + std::to_string(remainder) + " (" + std::to_string(quantity) + ")", 3);
        std::get<1>(contents[listing_index]) = quantity; print_cart(); return; // exit function since item_qty has surpassed the amount in stock
    }
    //-------------------------------
    // If cart_qty added with the user-specified quantity exceeds max_quantity (cart does not have to be full)
    // Adjust the quantity so it can fit into the cart
    // Example: 100-10=90 (max_quantity-cart_qty=quantity)(you need at least a quantity of 90 to fit everything into the cart)
    int cart_qty = database->get_integer_params("SELECT SUM(quantity) FROM cart_item WHERE cart_id = $1", { cart_id });
    if((cart_qty + quantity) > max_quantity) quantity = max_quantity - cart_qty;////neroshop::print(std::string("\033[0;33m") + std::string("Cart is full (max_quantity (") + std::to_string(max_quantity) + ") has been reached)\033[0m"); }
    // If item is already in the cart, just update the quantity and exit the function
    if(in_cart(listing_key) && quantity > 0) {
        int new_quantity = item_qty + quantity;
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND listing_key = $3", { std::to_string(new_quantity), cart_id, listing_key });
        neroshop::print(std::string("Already in cart: ") + item_name + " +" + std::to_string(quantity) + " (" + std::to_string(new_quantity) + ")", 3);
        std::get<1>(contents[listing_index]) = new_quantity; print_cart(); return;
    }
    // Add item to cart (cart_item)
    database->execute_params("INSERT INTO cart_item (cart_id, listing_key, quantity, seller_id) "
                             "VALUES ($1, $2, $3, $4)", 
    { cart_id, listing_key, std::to_string(quantity), seller_id });
    contents.emplace_back(listing_key, quantity, ""); // Save in memory as well
    neroshop::print(item_name + " (" + std::to_string(quantity) + ") added to cart", 3);
    print_cart();
}
////////////////////
void neroshop::Cart::add(const std::string& user_id, const neroshop::Product& item, int quantity) {
    add(user_id, item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::remove(const std::string& user_id, const std::string& listing_key, int quantity) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
}
////////////////////
void neroshop::Cart::remove(const std::string& user_id, const neroshop::Product& item, int quantity) {
    remove(user_id, item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::empty() {
    if(id.empty()) { neroshop::print("No cart found", 1); return; }
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    database->execute_params("DELETE FROM cart_item WHERE cart_id = $1;", { this->id });
    contents.clear();
}
////////////////////
void neroshop::Cart::change_quantity(const std::string& user_id, const neroshop::Product& item, int quantity) {
}
////////////////////
////////////////////
////////////////////
////////////////////
void neroshop::Cart::print_cart() {
    for (const auto& item : contents) { 
        std::cout << std::get<0>(item) << " (x" << std::get<1>(item) << "), sold by " << std::get<2>(item) << std::endl; 
    }
    std::cout << "cart_qty: " << get_quantity() << std::endl;
    std::cout << "cart_items (rows): " << contents.size() << std::endl;
}
////////////////////
////////////////////
////////////////////
void neroshop::Cart::set_id(const std::string& id) {
    this->id = id;
}

void neroshop::Cart::set_owner_id(const std::string& owner_id) {
    this->owner_id = owner_id;
}

void neroshop::Cart::set_contents(const std::vector<std::tuple<std::string, int, std::string>>& contents) {
    this->contents = contents;
}
////////////////////
////////////////////
////////////////////
std::string neroshop::Cart::get_id() const {
    return id;
}
////////////////////
std::string neroshop::Cart::get_owner_id() const {
    return owner_id;
}
////////////////////
////////////////////
int neroshop::Cart::get_max_items() {
    return max_items;
}
////////////////////
int neroshop::Cart::get_max_quantity() {
    return max_quantity;
}
////////////////////
int neroshop::Cart::get_quantity() const {
    int quantity = 0;
    for (const auto& item : contents) {
        quantity += std::get<1>(item);
    }
    return quantity;
}
////////////////////
int neroshop::Cart::get_contents_count() const {
    return contents.size();
}
////////////////////
std::size_t neroshop::Cart::get_listing_index(const std::string& listing_key) {
    auto it = std::find_if(contents.begin(), contents.end(), [&listing_key](const auto& tuple) {
        return std::get<0>(tuple) == listing_key;
    });
    return (it != contents.end()) ? std::distance(contents.begin(), it) : contents.size(); // return index if found, else size of vector
}
////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::Cart::is_empty() const {
    return contents.empty();
}
////////////////////
bool neroshop::Cart::is_full() const {
    int quantity = get_quantity();
    return ((contents.size() >= max_items) || (quantity >= max_quantity)); // either cart.contents has reached its capacity (max_items:10) or all items have a combined quantity of 100 (max_quantity:100)
}
////////////////////
bool neroshop::Cart::in_cart(const std::string& listing_key) const {
    for (const auto& item : contents) {
        if (std::get<0>(item) == listing_key) {
            return true;
        }
    }
    return false;
}
////////////////////
bool neroshop::Cart::in_cart(const neroshop::Product& item) const {
    return in_cart(item.get_id());
}
////////////////////
////////////////////
////////////////////
////////////////////
