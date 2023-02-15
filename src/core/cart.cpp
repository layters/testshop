#include "cart.hpp"

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
    neroshop::db::Sqlite3 * database = neroshop::db::Sqlite3::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Set the cart's id
    this->id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });
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
        std::pair<std::string, int> cart_item; // Create a std::pair object for each row
        for(int i = 0; i < sqlite3_column_count(stmt); i++) {
            std::string column_value = (sqlite3_column_text(stmt, i) == nullptr) ? "NULL" : reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
            if(i == 0) cart_item.first = column_value; // product id
            if(i == 1) cart_item.second = std::stoi(column_value); // quantity
        }
        contents.insert(cart_item);
        ////neroshop::print("loaded cart item (id: " + cart_item.first + ", qty: " + std::to_string(cart_item.second) + ")", 3);
    }
    /////////////////////////////
    // Update item quantities based on stock available
    for (auto const& [key, value] : contents) {
        std::string product_id = key; 
        int quantity = value;
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
            contents.erase(contents.find(product_id));
            continue; // skip this item since it is no longer in stock, and so we do not store it in the cart            
        }
        // Adjust item quantity to match the stock available
        if(quantity >= stock_available) quantity = stock_available;
        // Update the item's quantity (just to be sure it does not surpass the stock available)
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND product_id = $3", { std::to_string(quantity), this->id, product_id });        
        // Update item quantity in memory
        contents[product_id] = quantity;
        neroshop::print("loaded cart item (id: " + product_id + ", qty: " + std::to_string(quantity) + ")", 3);
    }
    _print();    
}
////////////////////
void neroshop::Cart::add(const std::string& user_id, const std::string& product_id, int quantity) {
    neroshop::db::Sqlite3 * database = neroshop::db::Sqlite3::get_database();
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
        neroshop::print(item_name + " is out of stock", 1);return;
    }
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
        contents[product_id] = quantity; _print(); return; // exit function since item_qty has surpassed the amount in stock
    }
    //-------------------------------
    // If cart_qty added with the user-specified quantity exceeds max_quantity (cart does not have to be full)
    // Adjust the quantity so it can fit into the cart
    // Example: 100-10=90 (max_quantity-cart_qty=quantity)(you need at least a quantity of 90 to fit everything into the cart)
    int cart_qty = database->get_integer_params("SELECT SUM(quantity) FROM cart_item WHERE cart_id = $1", { cart_id });
    if((cart_qty + quantity) > max_quantity) quantity = max_quantity - cart_qty;////neroshop::print(std::string("\033[0;33m") + std::string("Cart is full (max_quantity (") + std::to_string(max_quantity) + ") has been reached)\033[0m"); }
    //-------------------------------
    // If item is already in the cart, just update the quantity and exit the function
    if(in_cart(product_id) && quantity > 0) {
        int new_quantity = item_qty + quantity;
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND product_id = $3", { std::to_string(new_quantity), cart_id, product_id });
        neroshop::print(std::string("Already in cart: ") + item_name + " +" + std::to_string(quantity) + " (" + std::to_string(new_quantity) + ")", 3);
        contents[product_id] = new_quantity; _print(); return;
    }
    // Add item to cart (cart_item)
    database->execute_params("INSERT INTO cart_item (cart_id, product_id, quantity) "
                             "VALUES ($1, $2, $3)", 
    { cart_id, product_id, std::to_string(quantity) });
    contents.insert(std::make_pair(product_id, quantity)); // Save in memory as well
    neroshop::print(item_name + " (" + std::to_string(quantity) + ") added to cart", 3);
    _print();
}
////////////////////
void neroshop::Cart::add(const std::string& user_id, const neroshop::Item& item, int quantity) {
    add(user_id, item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::remove(const std::string& user_id, const std::string& product_id, int quantity) {
    neroshop::db::Sqlite3 * database = neroshop::db::Sqlite3::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
}
////////////////////
void neroshop::Cart::remove(const std::string& user_id, const neroshop::Item& item, int quantity) {
    remove(user_id, item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::empty(const std::string& user_id) {
    neroshop::db::Sqlite3 * database = neroshop::db::Sqlite3::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    std::string cart_id = database->get_text_params("SELECT uuid FROM cart WHERE user_id = $1", { user_id });
    database->execute_params("DELETE FROM cart_item WHERE cart_id = $1;", { cart_id });
    contents.clear();
}
////////////////////
void neroshop::Cart::change_quantity(const std::string& user_id, const neroshop::Item& item, int quantity) {
}
////////////////////
////////////////////
////////////////////
////////////////////
void neroshop::Cart::_print() {
    for (auto const& [key, value] : contents) { // https://stackoverflow.com/a/26282004
        std::cout << key << " (x" << value << ")" << std::endl; 
    }
    std::cout << "cart_qty: " << get_quantity() << std::endl;
    std::cout << "cart_items (rows): " << contents.size() << std::endl;
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
std::string neroshop::Cart::get_id() const {
    return id;
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
    for (auto const& [key, value] : contents) {
        quantity = quantity + value;
    }
    return quantity;
}
////////////////////
int neroshop::Cart::get_contents_count() const {
    return contents.size();
}
////////////////////
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
    return (contents.count(product_id) > 0);
}
////////////////////
bool neroshop::Cart::in_cart(const neroshop::Item& item) const {
    return in_cart(item.get_id());
}
////////////////////
////////////////////
////////////////////
////////////////////
