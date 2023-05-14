#include "cart.hpp"

#include "database/database.hpp"
#include "tools/logger.hpp"
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
////////////////////
// normal
////////////////////
void neroshop::Cart::load(const std::string& user_id) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Set the cart's id
    this->id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });
    // Set the cart's owner id
    this->owner_id = user_id;
    // Prepare (compile) statement
    std::string command = "SELECT product_id, quantity FROM cart_item WHERE cart_id = $1";
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
            if(i == 0) std::get<0>(cart_item) = column_value; // product id
            if(i == 1) std::get<1>(cart_item) = std::stoi(column_value); // quantity
            if(i == 2) std::get<2>(cart_item) = column_value; // seller id
        }
        contents.push_back(cart_item);
        ////neroshop::print("loaded cart item (id: " + std::get<0>(cart_item) + ", qty: " + std::to_string(std::get<1>(cart_item)) + ")", 3);
    }
    /////////////////////////////
    // Update item quantities based on stock available
    for (auto& cart_item : contents) {
        std::string product_id = std::get<0>(cart_item);
        int quantity = std::get<1>(cart_item);
        std::string item_name = database->get_text_params("SELECT name FROM products WHERE uuid = $1", { product_id });
        int stock_available = database->get_integer_params("SELECT quantity FROM listings WHERE product_id = $1 AND quantity > 0", { product_id });
        if(stock_available == 0) {
            neroshop::print(item_name + " is out of stock", 1);
            // Set item quantity to zero
            database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND product_id = $3", { std::to_string(0), this->id, product_id });
            // Remove the item from the user's cart
            database->execute_params("DELETE FROM cart_item WHERE cart_id = $1 AND product_id = $2", { this->id, product_id });
            neroshop::print(item_name + " (x" + std::to_string(quantity) + ") removed from cart", 1);
            // Remove the item from std::map
            contents.erase(std::remove_if(contents.begin(), contents.end(), [&](auto const& item){ return std::get<0>(item) == product_id; }), contents.end());
            continue; // skip this item since it is no longer in stock, and so we do not store it in the cart            
        }
        // Adjust item quantity to match the stock available
        if(quantity >= stock_available) quantity = stock_available;
        // Update the item's quantity (just to be sure it does not surpass the stock available)
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND product_id = $3", { std::to_string(quantity), this->id, product_id });        
        // Update item quantity in memory
        std::get<1>(cart_item) = quantity;
        neroshop::print("loaded cart item (id: " + product_id + ", qty: " + std::to_string(quantity) + ")", 3);
    }
    print_cart();
}
////////////////////
void neroshop::Cart::add(const std::string& user_id, const std::string& product_id, int quantity) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    if(quantity < 1) return;
    std::string item_name = database->get_text_params("SELECT name FROM products WHERE uuid = $1", { product_id }); // temporary
    // Get cart uuid
    std::string cart_id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });//std::cout << "cart_uuid: " << cart_id << std::endl;
    // Cart is full - exit function
    if(is_full()) { neroshop::print("Cart is full", 1); return; }
    // Out of stock - exit function
    int stock_available = database->get_integer_params("SELECT quantity FROM listings WHERE product_id = $1 AND quantity > 0", { product_id });
    if(stock_available == 0) {
        neroshop::print(item_name + " is out of stock", 1); return;
    }
    //-------------------------------
    // find the index of the tuple containing the specified product_id
    std::size_t product_index = get_product_index(product_id);
    //-------------------------------
    // Make sure user-specified quantity does not exceed amount in stock
    int item_qty = database->get_integer_params("SELECT quantity FROM cart_item WHERE cart_id = $1 AND product_id = $2", { cart_id, product_id });
    // If item quantity surpasses the amount in stock, set item quantity to the amount in stock
    if((item_qty + quantity) > stock_available) {
        neroshop::print("Only " + std::to_string(stock_available) + " " + item_name + "s left in stock", 1);
        int remainder = (stock_available - item_qty);
        if(remainder == 0) return; // If zero then there's no more that you can add to the cart
        quantity = item_qty + remainder; // Increase item quantity by whatever is left of the stock available
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND product_id = $3", { std::to_string(quantity), cart_id, product_id });
        neroshop::print(std::string("Already in cart: ") + item_name + " +" + std::to_string(remainder) + " (" + std::to_string(quantity) + ")", 3);
        std::get<1>(contents[product_index]) = quantity; print_cart(); return; // exit function since item_qty has surpassed the amount in stock
    }
    //-------------------------------
    // If cart_qty added with the user-specified quantity exceeds max_quantity (cart does not have to be full)
    // Adjust the quantity so it can fit into the cart
    // Example: 100-10=90 (max_quantity-cart_qty=quantity)(you need at least a quantity of 90 to fit everything into the cart)
    int cart_qty = database->get_integer_params("SELECT SUM(quantity) FROM cart_item WHERE cart_id = $1", { cart_id });
    if((cart_qty + quantity) > max_quantity) quantity = max_quantity - cart_qty;////neroshop::print(std::string("\033[0;33m") + std::string("Cart is full (max_quantity (") + std::to_string(max_quantity) + ") has been reached)\033[0m"); }
    // If item is already in the cart, just update the quantity and exit the function
    if(in_cart(product_id) && quantity > 0) {
        int new_quantity = item_qty + quantity;
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND product_id = $3", { std::to_string(new_quantity), cart_id, product_id });
        neroshop::print(std::string("Already in cart: ") + item_name + " +" + std::to_string(quantity) + " (" + std::to_string(new_quantity) + ")", 3);
        std::get<1>(contents[product_index]) = new_quantity; print_cart(); return;
    }
    // Add item to cart (cart_item)
    database->execute_params("INSERT INTO cart_item (cart_id, product_id, quantity) "
                             "VALUES ($1, $2, $3)", 
    { cart_id, product_id, std::to_string(quantity) });
    contents.emplace_back(product_id, quantity, ""); // Save in memory as well
    neroshop::print(item_name + " (" + std::to_string(quantity) + ") added to cart", 3);
    print_cart();
}
////////////////////
void neroshop::Cart::add(const std::string& user_id, const neroshop::Product& item, int quantity) {
    add(user_id, item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::remove(const std::string& user_id, const std::string& product_id, int quantity) {
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
std::size_t neroshop::Cart::get_product_index(const std::string& product_id) {
    auto it = std::find_if(contents.begin(), contents.end(), [&product_id](const auto& tuple) {
        return std::get<0>(tuple) == product_id;
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
bool neroshop::Cart::in_cart(const std::string& product_id) const {
    for (const auto& item : contents) {
        if (std::get<0>(item) == product_id) {
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
