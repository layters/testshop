#include "../include/cart.hpp"

////////////////////
neroshop::Cart::Cart() : id(0), max_items(10), max_quantity(100) {} // cart can only hold up to 10 unique items // cart items can only add up to 100 qty
////////////////////
neroshop::Cart::~Cart() {
    contents.clear(); // this should reset (delete) all cart items
#ifdef NEROSHOP_DEBUG
    std::cout << "cart deleted\n";
#endif
}
////////////////////
////////////////////
////////////////////
// normal
////////////////////
void neroshop::Cart::add(unsigned int item_id, int quantity) {
    // get the user_id from the cart
#if defined(NEROSHOP_USE_POSTGRESQL)     
    int user_id = DB::Postgres::get_singleton()->get_integer_params("SELECT user_id FROM cart WHERE id = $1;", { std::to_string(this->id) });
    if(user_id == 0) { add_to_guest_cart(item_id, quantity); return; }
    add_to_registered_user_cart(user_id, item_id, quantity);
#endif    
}
////////////////////
void neroshop::Cart::add(const neroshop::Item& item, int quantity) {
    add(item.get_id(), quantity);
}
////////////////////
void neroshop::Cart::remove(unsigned int item_id, int quantity) {
    // get the user_id from the cart
#if defined(NEROSHOP_USE_POSTGRESQL)     
    int user_id = DB::Postgres::get_singleton()->get_integer_params("SELECT user_id FROM cart WHERE id = $1;", { std::to_string(this->id) });
    if(user_id == 0) { remove_from_guest_cart(item_id, quantity); return; }
    remove_from_registered_user_cart(user_id, item_id, quantity);
#endif    
}
////////////////////
void neroshop::Cart::remove(const neroshop::Item& item, int quantity) {
    remove(item.get_id(), quantity);
}
////////////////////
////////////////////
////////////////////
bool neroshop::Cart::open() const {
    /*// cart data is stored locally on a user's device
    create_guest_cart(); // create cart table if not exist
    // a cart tied to an account (user_id) should be named: cart_user.db (e.g cart_layter.db) or??
    // cookies.sqlite3 - should not be loaded until user logs in or??
    std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    std::ifstream file(cart_file.c_str()); // no need to use f.close() as the file goes out of scope at the end of the function
    // if cookies.sqlite3 already exists        
    if(file.good()) {
        std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
        NEROSHOP_TAG_OUT std::cout << "\033[1;36m" << "cart found: \"" << cart_file << "\"\033[0m" << std::endl;
        // open cart db-
        neroshop::DB::SQLite3 db;
        if(!db.open(cart_file)) {neroshop::print(SQLITE3_TAG + std::string("could not open cookies.sqlite3")); return false;}
        ////db.execute("PRAGMA journal_mode = WAL;"); // to prevent database from being locked
        if(!db.table_exists("cart")) { // table Cart is missing either due to corruption or something
            neroshop::print(SQLITE3_TAG + std::string("table Cart is missing (Please delete cookies.sqlite3 and then restart the application)"));
            db.close();
            return false;
        }
        // create items based on item_ids stored in cookies.sqlite3
        int cart_items_count = db.row_count("cart"); // numbers of items currently in cart
        unsigned int last_cart_item = db.get_column_integer("cart ORDER BY id DESC LIMIT 1", "*"); // number of inserted rows in table cart //std::cout << "last inserted row in table cart: " << last_cart_item << std::endl;
        if(cart_items_count < 1) neroshop::print("Cart is empty");
        if(cart_items_count > 0) 
        {   // load items into the cart
            for(unsigned int i = 1; i <= last_cart_item; i++) {//for(unsigned int i = 1; i <= cart_items_count; i++) {
                unsigned int cart_item = db.get_column_integer("cart", "item_id", "id = " + std::to_string(i));
                if(cart_item == 0) continue; // item not in cart, so skip
                unsigned int item_qty = db.get_column_integer("cart", "item_qty", "item_id = " + std::to_string(cart_item));//neroshop::print("loaded cart item (id: " + std::to_string(cart_item) + ", qty: " + std::to_string(item_qty) + ")");// << std::endl;//std::cout << "found cart item with id: " << i << " (qty: " << item_qty + ")" << std::endl;
                neroshop::Item * item = new Item(cart_item);//Item item(cart_item);//item created on stack dies at end of for loop that is why it returns invalid values
                // if the cart is not empty, just load it, no need to add more of the same items again
                const_cast<Cart *>(this)->load(*item, item_qty);
            }
        }
        db.close();
    }
    if(!file.good()) { // if cookies.sqlite3 does not exist, create it
        if(!neroshop::Cart::create_guest_cart()) {neroshop::print(SQLITE3_TAG + std::string("failed to create cookies.sqlite3")); return false;}
    }
    return true;*/      
    return false;  
}
////////////////////
void neroshop::Cart::load(const neroshop::Item& item, unsigned int quantity) { // load pre-existing cart data
    // be sure item is in stock
    if(!item.in_stock()) 
    {   // remove item from cart if it is no longer in stock
        NEROSHOP_TAG_OUT std::cout << "\033[1;91m" << item.get_name() << " is out of stock" << "\033[0m" << std::endl; // An item that was in your cart is now out of stock
        const_cast<neroshop::Item&>(item).set_quantity(0, 0); // set quantity to 0
        remove_db(item.get_id()); // remove item_id from table Cart
        NEROSHOP_TAG_OUT std::cout << "\033[1;91m" << item.get_name() << " (" << quantity << ") removed from cart" << "\033[0m" << std::endl;
        return; // exit function
    }
    unsigned int stock_qty = item.get_stock_quantity(); // get stock quantity
    if(quantity >= stock_qty) quantity = stock_qty; // quantity cannot be more than what is in stock
    // load the item and quantity ...
    std::shared_ptr<neroshop::Item> cart_item(&const_cast<neroshop::Item&>(item));
    this->contents.push_back(cart_item); // store existing item
    const_cast<neroshop::Item&>(item).set_quantity(quantity, 0); // save existing item_qty
    NEROSHOP_TAG_OUT std::cout << "\033[1;32m" << item.get_name() << " (id: " << item.get_id() << ", qty: " << quantity << ") has been loaded into cart" << "\033[0m" << std::endl;
}
////////////////////
void neroshop::Cart::add_to_guest_cart(unsigned int item_id, int quantity) {
    // get item name
#if defined(NEROSHOP_USE_POSTGRESQL)     
    std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
    // check if item is in database
    bool item_registered = (DB::Postgres::get_singleton()->get_text_params("SELECT EXISTS(SELECT id FROM item WHERE id = $1);", { std::to_string(item_id) }) == "t") ? true : false;
    if(!item_registered) {
        neroshop::print("\033[1;91m" + item_name + " is not registered in the database");
        return;
    }
    // check if the item is in stock
    bool item_in_stock = (DB::Postgres::get_singleton()->get_text_params("SELECT EXISTS(SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0)", { std::to_string(item_id) }) == "t") ? true : false;
    if(!item_in_stock) {
        neroshop::print("\033[1;91m" + item_name + " is out of stock"); 
        return;
    }
    // debug
    unsigned int cart_id = this->id;
    std::cout << "cart_id: " << cart_id << std::endl;
    int item_qty = Item::get_quantity(item_id, cart_id); // get item_qty (previously in cart)
    int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) }); // get stock_qty (in inventory)
    if(quantity >= stock_qty) quantity = stock_qty - item_qty; // make sure quantity is not more than the amount in stock
    // if item_qty is more than stock amount
    if(item_qty >= stock_qty) {
        neroshop::Item::set_quantity(item_id, stock_qty, cart_id);//update(item.get_id(), stock_qty); // set item_qty to stock_qty
        NEROSHOP_TAG_OUT std::cout << "Only " << "\033[1;91m" << stock_qty << "\033[0m " << item_name << "s left in stock" << std::endl;//NEROSHOP_TAG_OUT std::cout << "\033[0;33mYou have exceeded the amount in stock\033[0m" << std::endl;
        return; // exit function since item_qty has surpassed the amount in stock
    }
    // make sure cart is not full, else exit function
    if(is_full()) {
        std::string reason = (contents.size() >= max_items) ? "you can only add 10 items (unique) to cart" : "max_quantity (100) has been exceeded";
        neroshop::print("\033[0;33mCart is full (" +  reason + ")");
        return; // exit function if full
    }
    // quantity cannot be less than one
    if(quantity <= 0) quantity = 1;    
    // quantity cannot exceed max_quantity, so exit function (lets say that the quantity is 4294967280 - this is not acceptable)
    if(quantity > max_quantity) {NEROSHOP_TAG_OUT std::cout << "Quantity is too high (max_quantity (" << max_quantity << ") has been exceeded)" << "\033[0m" << std::endl; return;}
    // if cart_qty added with the new quantity exceeds max_quantity (cart does not have to be full)
    unsigned int cart_qty = get_total_quantity();//_db();
    if((cart_qty + quantity) > max_quantity)
    {
        NEROSHOP_TAG_OUT std::cout << "\033[0;33m" << "Cart is full (max_quantity (" << max_quantity << ") has been reached)" << "\033[0m" << std::endl;// : " << quantity << " " << const_cast<neroshop::Item&>(item).get_name() << "s ("<< get_quantity() << "+" << quantity << "=" << (get_quantity() + quantity) << ")" << std::endl; //         std::cout << "You can only have a quantity of " << max_quantity << " items in your cart" << std::endl;
        // adjust the quantity so it can fit
        // 100-10=90 (max_quantity-items_count=quantity)(you need at least a quantity of 90 to fit everything into the cart)
        quantity = max_quantity - cart_qty;
    }
    // if item is already in cart
    if(in_cart(item_id))//if(in_cart(item)) 
    {
        neroshop::Item::set_quantity(item_id, item_qty + quantity, cart_id);// increase item quantity by a user-specified amount//update(item.get_id(), item_qty + quantity);
        item_qty = neroshop::Item::get_quantity(item_id, cart_id); // get updated quantity
        NEROSHOP_TAG_OUT std::cout << "\033[0;32m" << "Already in cart: " << item_name << " +" << quantity << " (" << item_qty << ")" << "\033[0m" << std::endl;
    }
    // if item is not already in cart
    if(!in_cart(item_id))//if(!in_cart(item)) 
    {
        add_db(item_id); // add item to table Cart
        std::shared_ptr<neroshop::Item> cart_item = std::make_shared<neroshop::Item>(item_id);
        contents.push_back(cart_item);// add item to cart.contents
        cart_item->set_quantity(item_qty + quantity, cart_id);// increase item quantity by a user-specified amount
        NEROSHOP_TAG_OUT std::cout << "\033[1;32m" << item_name << " (" << quantity << ") added to cart" << "\033[0m" << std::endl;
    }
#ifdef NEROSHOP_DEBUG
    NEROSHOP_TAG_OUT std::cout << "\033[0;97m" << "Total items in cart: " << get_items_count() << "\033[0m" << std::endl;
#endif   
#endif 
}
////////////////////
void neroshop::Cart::add_to_guest_cart(const neroshop::Item& item, int quantity) { // good!
    add_to_guest_cart(item.get_id(), quantity);
}
//////////////////// blue: << "\033[0;34m"
void neroshop::Cart::remove_from_guest_cart(unsigned int item_id, int quantity) {

}
////////////////////
void neroshop::Cart::remove_from_guest_cart(const neroshop::Item& item, int quantity) {
    // if item is not in cart, exit function since there is nothing to remove
    if(!in_cart(item.get_id())) return;
    // make sure quantity is not more than item_quantity (cannot remove more than that which is in the cart)
    unsigned int item_qty = item.get_quantity(0);
    if(quantity >= item_qty) quantity = item_qty;
    // if item is already in cart
    // if item_quantity is more than zero (0), reduce item_quantity by 1 or whatever the specified quantity is
    if(item_qty > 0) 
    {
        const_cast<neroshop::Item&>(item).set_quantity(item_qty - quantity, 0);//(item.get_quantity() - quantity);
        item_qty = item.get_quantity(0); // item_qty has been updated so get the updated item_qty
        NEROSHOP_TAG_OUT std::cout << "\033[0;91m" << item.get_name() << " (" << quantity << ") removed from cart" << "\033[0m" << std::endl;
    }
    // if item_quantity is zero (0), delete item from cart
    if(item_qty <= 0) 
    {
        const_cast<neroshop::Item&>(item).set_quantity(0, 0); // first, make sure item quantity is exactly zero (0)
        // now remove it from cart.contents
        for(int i = 0; i < contents.size(); i++) {
            if(item.get_id() == contents[i]->get_id()) {contents.erase(contents.begin()+i);}//std::cout << "cart [" << i << "]: " << contents[i]->get_name() << std::endl;
        }
        // remove the item from the database as well
        remove_db(item.get_id());
    }
    // if cart is empty, clear the contents: if(is_empty()) empty();
#ifdef NEROSHOP_DEBUG
    NEROSHOP_TAG_OUT std::cout << "\033[0;97m" << "Total items in cart: " << get_items_count() << "\033[0m" << std::endl;
#endif            
    if(is_empty()) neroshop::print("Cart is empty");
}
////////////////////
/*void neroshop::Cart::remove_from_guest_cart(unsigned int index, int quantity) {
    if(index > (contents.size()-1)) throw std::runtime_error(std::string("\033[0;31m") + "neroshop::Cart::remove(unsigned int, unsigned int): attempting to access invalid index" + std::string("\033[0m"));
    remove_from_guest_cart(*contents[index], quantity); // remove item at an index
}*/
////////////////////
void neroshop::Cart::empty() {
    /*contents.clear();
    // clear the sqlite Cart table as well
    std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);
    if(!db.table_exists("cart")) {db.close(); return;}
    db.truncate("cart");
    //db.vacuum(); // probably not necessary since cookies.sqlite3 is smaller than the main db and is stored locally on the user's device
    db.close();*/
}
////////////////////
////////////////////
////////////////////
void neroshop::Cart::change_quantity(const neroshop::Item& item, int quantity) {
    if(!in_cart(item.get_id())) return; // you can only change the qty of items that are already inside of the cart    
    // check if item is in stock
    if(!item.in_stock()) {NEROSHOP_TAG_OUT std::cout << "\033[1;91m" << item.get_name() << " is out of stock" << "\033[0m" << std::endl; return;}
    int stock_qty = item.get_stock_quantity(); // get stock quantity
    if(quantity >= stock_qty) {quantity = stock_qty; NEROSHOP_TAG_OUT std::cout << "Only " << "\033[1;91m" << stock_qty << "\033[0m " << item.get_name() << "s left in stock" << std::endl;} // make sure quantity is not more than the amount in stock
    // if item quantity is the same as the specified amount, exit function
    unsigned int item_qty = item.get_quantity(0);
    if(quantity == item_qty) return; // there is really nothing to change here since item_qty and specified quantity are both the same value
    // if cart is full, but user still wants to increase the item_qty
    if(is_full() && (quantity > 0)) {NEROSHOP_TAG_OUT std::cout << "\033[0;33m" << "Cart is full, and you still want to add more shit to it, bruh?" << "\033[0m" << std::endl;return;}
    if(quantity < 0) quantity = 0; // quantity cannot be negative
    if(quantity > max_quantity) quantity = max_quantity; // quantity cannot exceed max_quantity
    // if cart_qty added with the new quantity exceeds max_quantity (cart does not have to be full)
    unsigned int cart_qty = get_total_quantity();
    if((cart_qty + quantity) > max_quantity)
    {
        // adjust the quantity so it can fit
        // (100+1)-2=99 ((max_quantity+item_qty)-items_count=quantity)(you need at least a quantity of 99 to fit everything into the cart)
        quantity = (max_quantity + item_qty) - cart_qty;
        // cart can no longer fit any more items
        NEROSHOP_TAG_OUT std::cout << "\033[0;33m" << "Cart is full (cannot fit any more items)" << "\033[0m" << std::endl;
        // cart is full but the cart item's previous quantity is higher than the new quantity
        //if(item.get_quantity() > quantity) {return;} // cart is already full, so no need to increase the qty. Exit the function!
        //return; // exit the function
    }
    // if quantity of item is set to 0 or less, remove the item from the cart (remove() will auto set the quantity to 0)
    if(quantity <= 0) { remove_from_guest_cart(item, max_quantity); return;} // remove all of it from the cart
    // with all obstacles out of the way, change the qty of the item
    const_cast<neroshop::Item&>(item).set_quantity(quantity, 0); // changes quantity in database as well
#ifdef NEROSHOP_DEBUG
    neroshop::print(item.get_name() + " quantity has changed to \033[0;36m" + std::to_string(quantity) + "\033[0m");
    NEROSHOP_TAG_OUT for(int i = 0; i < contents.size(); i++) {
        item_qty = neroshop::Item::get_quantity(contents[i]->get_id(), 0);
        std::string item_name = contents[i]->get_name();
        std::cout << "Cart [" << i << "]: " << item_name << " (" << item_qty << ")" << std::endl;
    }
    //NEROSHOP_TAG_OUT std::cout << "\033[0;97m" << "Total items in cart: " << get_items_count() << "\033[0m" << std::endl;
#endif
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
double neroshop::Cart::get_seller_subtotal_price(unsigned int seller_id) const { 
    double seller_price = 0.00;
    for(int i = 0; i < contents.size(); i++) {
        seller_price += contents[i]->get_quantity(0) * contents[i]->get_seller_price(seller_id); // if seller_id is 0, it will return a random seller's price
    }
    return seller_price;
}
////////////////////
double neroshop::Cart::get_subtotal_price() const {
    double items_price = 0.00; // price of all cart items combined
    for(int i = 0; i < contents.size(); i++) {
        items_price += contents[i]->get_quantity(0) * contents[i]->get_price();
    }
    return items_price;
}
////////////////////
////////////////////
unsigned int neroshop::Cart::get_total_quantity() const {
    ///////////////////
    // this is faster (I think, since we don't use any loops here)
    /*std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);
    int items_count = db.get_column_integer("cart", "sum(item_qty) AS cart_qty"); // DB::Postgres::get_singleton()->get_integer("SELECT SUM(item_qty) FROM Cart;");
    db.close();
    return items_count;*/
    /////////////////////////////
    // this is slower (I think, since it opens and closes the db at least 10 times)
    /*unsigned int items_count = 0; // quantity of all cart items combined
    for(int i = 0; i < contents.size(); i++) {
        items_count += contents[i]->get_quantity();
    }
    std::cout << "items_count: " << items_count << std::endl;   
    return items_count;*/
    return 0;
}
////////////////////
unsigned int neroshop::Cart::get_total_quantity_db() { // faster than neroshop::Cart::get_total_quantity() since this does not open the db file multiple times, but only one time
    /*std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);
    if(!db.table_exists("cart")) {db.close(); return 0;}
    ///////
    unsigned int row_count = db.row_count("cart");
    unsigned int items_count = 0;
    if(row_count < 1) return 0;
    for(unsigned int i = 1; i <= row_count; i++) {
        unsigned int item_qty = db.get_column_integer("cart", "item_qty", "item_id = " + std::to_string(i));
        items_count += item_qty;
    }
    db.close();
    return items_count;*/
    return 0;
}
////////////////////
double neroshop::Cart::get_total_weight() const {
    ///////////////////////////// https://stackoverflow.com/questions/30909020/postgresql-how-to-multiply-two-columns-and-display-result-in-same-query    // https://www.alphacodingskills.com/postgresql/notes/postgresql-operator-multiply.php
    double items_weight = 0.0; // weight of all cart items combined
    for(int i = 0; i < contents.size(); i++) {
        items_weight += contents[i]->get_quantity(0) * contents[i]->get_weight(); // weight should be determined by (items_weight * quantity)  // the more the quantity, the bigger the weight
    }
    //std::cout << "items_weight: " << items_weight << std::endl;
    return items_weight;
}
////////////////////
double neroshop::Cart::get_seller_total_discount(unsigned int seller_id) const {
    double seller_discount = 0.00;
    for(int i = 0; i < contents.size(); i++) {
        seller_discount += contents[i]->get_seller_discount(seller_id);
    }
    return seller_discount;
}
////////////////////
double neroshop::Cart::get_total_discount() const { // ?
    double items_discount = 0.00; // discount of all cart items combined
    for(int i = 0; i < contents.size(); i++) {
        items_discount += 0.00;//contents[i]->get_discount(i);//discounts are determined by sellers
    }
    return items_discount;
}
////////////////////
////////////////////
unsigned int neroshop::Cart::get_items_count() const {
    return get_total_quantity();
}
////////////////////
unsigned int neroshop::Cart::get_contents_count() const {
    return contents.size();
}
////////////////////
unsigned int get_contents_count_db() { // returns number of rows in table Cart
    /*std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);
    if(!db.table_exists("cart")) {db.close(); return 0;}
    int row_count = db.row_count("cart");
    db.close();
    return row_count;*/
    return 0;
}
////////////////////
////////////////////
////////////////////
neroshop::Item * neroshop::Cart::get_item(unsigned int index) const {
    if(index > (contents.size() - 1)) throw std::out_of_range(std::string(std::string("\033[0;31m") + "(neroshop::Cart::get_item): attempting to access invalid index" + std::string("\033[0m")).c_str());//std::cerr << "\033[0;31m" << "neroshop::Cart::get_item(): attempting to access invalid index" << "\033[0m" << std::endl;//return nullptr;
    return contents[index].get();
}
////////////////////
std::vector<std::shared_ptr<neroshop::Item>> neroshop::Cart::get_contents_list() const {
    return contents;
}
////////////////////
unsigned int neroshop::Cart::get_id() const {
    return id;
}
////////////////////
unsigned int neroshop::Cart::get_owner_id() const {
    if(this->id == 0) return 0; // invalid cart_id means invalid user_id
#if defined(NEROSHOP_USE_POSTGRESQL)     
    int user_id = DB::Postgres::get_singleton()->get_integer_params("SELECT user_id FROM cart WHERE id = $1;", { std::to_string(this->id) });
    std::cout << "cart belongs to user_id: " << user_id << std::endl; // temp
    return user_id;
#endif    
    return 0;
}
////////////////////
unsigned int neroshop::Cart::get_owner_id(unsigned int cart_id) {
    if(cart_id == 0) return 0;
#if defined(NEROSHOP_USE_POSTGRESQL)     
    int user_id = DB::Postgres::get_singleton()->get_integer_params("SELECT user_id FROM cart WHERE id = $1;", { std::to_string(cart_id) });
    std::cout << "cart belongs to user_id: " << user_id << std::endl; // temp
    return user_id;
#endif    
    return 0;
}
////////////////////
unsigned int neroshop::Cart::get_max_items() const {
    return max_items;
}
////////////////////
unsigned int neroshop::Cart::get_max_quantity() const {
    return max_quantity;
}
////////////////////
////////////////////
////////////////////
bool neroshop::Cart::is_empty() const {
    return contents.empty();//return (db.row_count("cart") == 0);
}
////////////////////
bool neroshop::Cart::is_full() const {
    return ((contents.size() >= max_items) || (get_total_quantity() >= max_quantity)); // either cart.contents has reached its capacity (max_items:10) or all items have a combined quantity of 100 (max_quantity:100)
}
////////////////////
bool neroshop::Cart::in_cart(unsigned int item_id) const {//(const neroshop::Item& item) const {
    //return (std::find(contents.begin(), contents.end(), &const_cast<neroshop::Item&>(item)) != contents.end());
    /*std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);
    if(!db.table_exists("cart")) {db.close(); return false;}
    int cart_item = db.get_column_integer("cart", "item_id", "item_id = " + std::to_string(item_id));
    if(cart_item == 0) return false; // 0 means item was never registered
    if(cart_item != item_id) return false; // if cart_item_id does not match item_id
    //std::cout << "item with id: " << item_id << " is inside cart" << std::endl;
    db.close();*/
    return true;
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::Cart::create_guest_cart() {
    /*std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);////db.execute("PRAGMA journal_mode = WAL;"); // this may reduce the incidence of SQLITE_BUSY errors (such as database being locked) // https://www.sqlite.org/pragma.html#pragma_journal_mode
    // one cart per user (guest) on each device // after each order, cart will be emptied
    if(!db.table_exists("cart")) {
        db.table("cart");
        db.execute("ALTER TABLE cart ADD COLUMN item_id INTEGER;");
        db.execute("ALTER TABLE cart ADD COLUMN item_qty INTEGER;");
        db.execute("ALTER TABLE cart ADD COLUMN item_price INTEGER;");//"REAL"); // for money, integer is the most heavily suggested in the sqlite commu :/
        db.execute("ALTER TABLE cart ADD COLUMN item_weight REAL;");
        db.index("cart_item_ids_index", "cart", "item_id");//db.execute("CREATE UNIQUE INDEX cart_item_ids_index ON Cart (item_id);");
    }
    db.close();*/
    return true;
}
////////////////////
//bool create_offline_db() {} // sqlite
////////////////////
void neroshop::Cart::add_db(unsigned int item_id) {
    /*std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);
    if(!db.table_exists("cart")) {db.close(); return;}
    db.insert("cart", "item_id, item_qty",
        std::to_string(item_id) + ", " + std::to_string(0) //+ ", " + std::to_string(item_price) + ", " + //std::to_string() + ", " + 
    );
    db.close();*/
}
////////////////////
void neroshop::Cart::remove_db(unsigned int item_id) {
    /*std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
    neroshop::DB::SQLite3 db(cart_file);
    if(!db.table_exists("cart")) {db.close(); return;}
    db.drop("cart", "item_id = " + std::to_string(item_id)); // deletes column where id=2
    //db.vacuum();
    //NEROSHOP_TAG_OUT std::cout << "item " << item_id << " has been removed from cookies.sqlite3" << std::endl;
    db.close();*/
} 
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// checks whether cart exists or not - should be changed to has_cart(int)
bool neroshop::Cart::load_cart(int user_id) {
    // check whether cart exists or not
#if defined(NEROSHOP_USE_POSTGRESQL)     
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });
    if(cart_id == 0) {
        // all users should have a cart at the time of registration
        neroshop::print("No cart found on account", 1);
        return false;
    }
    /////////////////////////////
    // set the cart's id if it is valid
    this->id = cart_id;
    /////////////////////////////
    // check if cart is empty or not
    int cart_items_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) }); // numbers of items currently in cart
    if(cart_items_count < 1) {
        neroshop::print("Cart is empty");
        return true; // nothing to load but cart exists, so return true
    }
    ///////////////////////////// store the code below in load_cart()
    // fetch cart items FROM cart_item WHERE cart_id = $1
    std::string command = "SELECT * FROM cart_item WHERE cart_id = $1";
    std::vector<const char *> param_values = { std::to_string(cart_id).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    int rows = PQntuples(result); //if(rows < 1) {PQclear(result); /*DB::Postgres::get_singleton()->finish();*/ return;}    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Cart::load_cart(): You have no items your cart", 2);        
        PQclear(result);
        //DB::Postgres::get_singleton()->finish();//exit(1);
        return false; // exit so we don't double free "result"
    }
    /////////////////////////////
    // create items based on item_ids stored in table: cart
    for(int i = 0; i < rows; i++) {
        int item_id = std::stoi(PQgetvalue(result, i, 2));
        int item_qty = std::stoi(PQgetvalue(result, i, 3));
        //if(item_id == 0) continue; // skip any invalid items - prolly not necessary        
        // make sure that the item is in stock
        bool in_stock = (DB::Postgres::get_singleton()->get_text_params("SELECT EXISTS (SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0)", { std::to_string(item_id) }) == "t") ? true : false;//std::cout << "item (id: " << item_id << ") in stock: " << ((in_stock) ? "true" : "false") << std::endl;
        if(!in_stock) { // An item that was in your cart is now out of stock
            std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
            neroshop::print(item_name + " is out of stock", 1);
            // set item quantity to zero
            DB::Postgres::get_singleton()->execute_params("UPDATE cart_item SET item_qty = $1 WHERE cart_id = $2 AND item_id = $3", { std::to_string(0), std::to_string(cart_id), std::to_string(item_id) });
            // remove the item from the user's cart
            DB::Postgres::get_singleton()->execute_params("DELETE FROM cart_item WHERE cart_id = $1 AND item_id = $2", { std::to_string(cart_id), std::to_string(item_id) });
            neroshop::print(item_name + " (x" + std::to_string(item_qty) + ") removed from cart", 1);
            continue; // skip this item since it is no longer in stock, and so we do not store it in the cart
        }
        // get the item's stock amount
        int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) });//std::cout << "stock quantity: " << stock_qty << std::endl;
        // item quantity cannot be more than what's in stock
        if(item_qty >= stock_qty) item_qty = stock_qty;
        // update the item's quantity (just to be sure it is not more than whats in stock)
        DB::Postgres::get_singleton()->execute_params("UPDATE cart_item SET item_qty = $1 WHERE cart_id = $2 AND item_id = $3", { std::to_string(item_qty), std::to_string(cart_id), std::to_string(item_id) });        
        // create the item (object) then store it in the cart(for later use) - it would be better to store the item_ids rather than the item object
        std::shared_ptr<neroshop::Item> item = std::make_shared<neroshop::Item>(item_id);//neroshop::Item * item = new Item(item_id);
        //if(std::find(contents.begin(), contents.end(), item) == contents.end()) {
            contents.push_back(item);
            neroshop::print("loaded cart item (id: " + std::to_string(item_id) + ", qty: " + std::to_string(item_qty) + ")");//NEROSHOP_TAG_OUT std::cout << "\033[1;32m" << item.get_name() << " (id: " << item.get_id() << ", qty: " << quantity << ") has been loaded into cart" << "\033[0m" << std::endl;
        //}
    }
    /////////////////////////////
    PQclear(result);
    /////////////////////////////
    return true;
#endif    
    return true;
}
////////////////////
////////////////////
void neroshop::Cart::add_to_registered_user_cart(int user_id, unsigned int item_id, int quantity) {
#if defined(NEROSHOP_USE_POSTGRESQL)     
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////    
    // if quantity is zero or negative then there's nothing to add
    if(quantity < 1) return;
    // make sure cart is not full, else exit function
    // ERROR FOUND HERE!!!!!!!!!!!
    if(is_full(user_id)) {
        neroshop::print(std::string("\033[0;33m") + "Cart is full (" + ((contents.size() >= max_items) ? "you can only add 10 items (unique) to cart)" : "max_quantity (100) has been exceeded)"));
        return; // exit function if full
    }        
    // fetch cart_id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });    
    if(cart_id == 0) {
        neroshop::print("No cart found on account", 1);
        return; // exit function
    }
    // remove this
    std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
    if(item_name.empty()) item_name = "???";
    // check if item is in database    
    ////int item_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM item WHERE id = $1", { std::to_string(item_id) });//if(!item.is_registered()) {NEROSHOP_TAG_OUT std::cout << "\033[1;91m" << item.get_name() << " is not registered in the database" << "\033[0m" << std::endl;return;}
    if(item_id == 0) {
        neroshop::print(item_name + " has not been registered");
        return; // exit function
    }
    // check if the item is in stock
    int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) });
    if(stock_qty == 0) {
        neroshop::print(item_name + " is out of stock", 1);
        return; // exit function
    }
    // get item_qty (previously in cart)
    int item_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT item_qty FROM cart_item WHERE cart_id = $1 AND item_id = $2", { std::to_string(cart_id), std::to_string(item_id) });
    if(quantity >= stock_qty) quantity = stock_qty - item_qty; // make sure quantity is not more than the amount in stock
    // if item_qty is more than stock amount
    if(item_qty >= stock_qty) {
        // set item_qty to stock_qty
        DB::Postgres::get_singleton()->execute_params("UPDATE cart_item SET item_qty = $1 WHERE cart_id = $2 AND item_id = $3", { std::to_string(stock_qty), std::to_string(cart_id), std::to_string(item_id) });
        neroshop::print("Only " + std::to_string(stock_qty) + " " + item_name + "s left in stock", 1);
        return; // exit function since item_qty has surpassed the amount in stock
    }
    // if cart_qty added with the new quantity exceeds max_quantity (cart does not have to be full)
    int cart_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT SUM(item_qty) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) });//get_total_quantity(user_id);
    if((cart_qty + quantity) > max_quantity)
    {
        neroshop::print(std::string("\033[0;33m") + std::string("Cart is full (max_quantity (") + std::to_string(max_quantity) + ") has been reached)\033[0m");// : " << quantity << " " << const_cast<neroshop::Item&>(item).get_name() << "s ("<< get_quantity() << "+" << quantity << "=" << (get_quantity() + quantity) << ")" << std::endl; //         std::cout << "You can only have a quantity of " << max_quantity << " items in your cart" << std::endl;
        // adjust the quantity so it can fit
        // 100-10=90 (max_quantity-items_count=quantity)(you need at least a quantity of 90 to fit everything into the cart)
        quantity = max_quantity - cart_qty;
    }
    // if item is already in cart
    if(in_cart(user_id, item_id))//if(in_cart(item)) 
    {
        // increase item quantity by a user-specified amount
        DB::Postgres::get_singleton()->execute_params("UPDATE cart_item SET item_qty = $1 WHERE cart_id = $2 AND item_id = $3", { std::to_string(item_qty + quantity), std::to_string(cart_id), std::to_string(item_id) });
        // get updated quantity
        item_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT item_qty FROM cart_item WHERE cart_id = $1 AND item_id = $2", { std::to_string(cart_id), std::to_string(item_id) });
        NEROSHOP_TAG_OUT std::cout << "\033[0;32m" << "Already in cart: " << item_name << " +" << quantity << " (" << item_qty << ")" << "\033[0m" << std::endl;
    }
    // if item is not already in cart
    if(!in_cart(user_id, item_id))//if(!in_cart(item)) 
    {
        // get item original price and weight
        double price = DB::Postgres::get_singleton()->get_double_params("SELECT price FROM item WHERE id = $1", { std::to_string(item_id) });
        float weight = DB::Postgres::get_singleton()->get_real_params("SELECT weight FROM item WHERE id = $1", { std::to_string(item_id) });
        // add item to table Cart
        //if(std::find(contents.begin(), contents.end(), &const_cast<neroshop::Item&>(item)) == contents.end()) {
            insert_into(user_id, item_id, quantity, price, weight);//item_qty + quantity// increase item quantity by a user-specified amount
            contents.push_back(std::make_shared<neroshop::Item>(item_id));// add item to cart.contents
            NEROSHOP_TAG_OUT std::cout << "\033[1;32m" << item_name << " (" << quantity << ") added to cart" << "\033[0m" << std::endl;  
        //}
    }
#ifdef NEROSHOP_DEBUG
    NEROSHOP_TAG_OUT std::cout << "\033[0;97m" << "Total items in cart: " << get_total_quantity(user_id) << "\033[0m" << std::endl;
#endif    
    ////////////////////
    //DB::Postgres::get_singleton()->finish();
#endif    
}
////////////////////
void neroshop::Cart::add_to_registered_user_cart(int user_id, const neroshop::Item& item, int quantity) {
    add_to_registered_user_cart(user_id, item.get_id(), quantity);
}
////////////////////
////////////////////
void neroshop::Cart::remove_from_registered_user_cart(int user_id, unsigned int item_id, int quantity) {
}
////////////////////
void neroshop::Cart::remove_from_registered_user_cart(int user_id, const neroshop::Item& item, int quantity) {
    remove_from_registered_user_cart(user_id, item.get_id(), quantity);
}
////////////////////
////////////////////
void neroshop::Cart::create_cart_item_table() {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    if(!DB::Postgres::get_singleton()->table_exists("cart_item")) {
        DB::Postgres::get_singleton()->create_table("cart_item");
        DB::Postgres::get_singleton()->execute("ALTER TABLE cart_item ADD COLUMN cart_id integer REFERENCES cart(id);");
        DB::Postgres::get_singleton()->execute("ALTER TABLE cart_item ADD COLUMN item_id integer REFERENCES item(id);");
        DB::Postgres::get_singleton()->execute("ALTER TABLE cart_item ADD COLUMN item_qty integer;");// DEFAULT 1;");//
        DB::Postgres::get_singleton()->execute("ALTER TABLE cart_item ADD COLUMN item_price numeric;");// original price, not seller price
        DB::Postgres::get_singleton()->execute("ALTER TABLE cart_item ADD COLUMN item_weight real;");
    }
#endif    
}
////////////////////
void neroshop::Cart::create_table() {
    create_cart_item_table();
}
////////////////////
void neroshop::Cart::insert_into(int user_id, int item_id, int item_qty, double item_price, float item_weight) {
#if defined(NEROSHOP_USE_POSTGRESQL)     
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////
    // begin transaction
    ////DB::Postgres::get_singleton()->execute("BEGIN;");
    // create a savepoint
    ////DB::Postgres::get_singleton()->execute("SAVEPOINT before_add_to_cart_savepoint;");
    // create table "cart_item" if not exists
    create_table();
    // fetch cart_id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });
    if(cart_id == 0) { neroshop::print("No cart found on account", 1); return; }//DB::Postgres::get_singleton()->execute("ROLLBACK TO before_add_to_cart_savepoint;");
    // add item to cart (if not already in cart)
    DB::Postgres::get_singleton()->execute_params("INSERT INTO cart_item (cart_id, item_id, item_qty, item_price, item_weight) "
        "VALUES ($1, $2, $3, $4, $5)", { std::to_string(cart_id), std::to_string(item_id), std::to_string(item_qty), std::to_string(item_price), std::to_string(item_weight) });
    // end transaction
    ////DB::Postgres::get_singleton()->execute("COMMIT;");
    ////////////////////
    //DB::Postgres::get_singleton()->finish();
#endif    
}
////////////////////
void neroshop::Cart::delete_from(int user_id, int item_id) {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch cart_id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });
#endif    
}
////////////////////
////////////////////
void neroshop::Cart::empty(int user_id) {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch cart_id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });
    // delete everything from user's cart
    DB::Postgres::get_singleton()->execute_params("DELETE FROM cart_item WHERE cart_id = $1;", { std::to_string(cart_id) });
#endif    
    // clear the contents from the vector as well
    contents.clear();
}
////////////////////
////////////////////
int neroshop::Cart::get_total_quantity(int user_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });    
    // get sum of all item_qty FROM cart_item WHERE cart_id = $1
    int total_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT SUM(item_qty) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) });
    return total_qty;
#endif    
    return 0;
}
////////////////////
double neroshop::Cart::get_subtotal_price(int user_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });
    // get sum of all (item_price * item_qty)  FROM cart_item WHERE cart_id = $1
    double subtotal_price = DB::Postgres::get_singleton()->get_double_params("SELECT SUM(item_price * item_qty) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) });    
    return subtotal_price;
#endif    
    return 0.0;
}
////////////////////
float neroshop::Cart::get_total_weight(int user_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });
    // get sum of all (item_weight * item_qty) FROM cart_item WHERE cart_id = $1
    float total_weight = DB::Postgres::get_singleton()->get_real_params("SELECT SUM(item_weight * item_qty) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) });
    return total_weight;
#endif    
    return 0.0f;
}
////////////////////
////////////////////
////////////////////
bool neroshop::Cart::is_empty(int user_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });
    // count number of items in cart_item WHERE cart_id = $1
    int total_items = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) });
    return (total_items == 0);
#endif    
    return false;
}
////////////////////
bool neroshop::Cart::is_full(int user_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });//std::cout << "cart_id: " << cart_id << std::endl;
    // 100 item_qty max
    int total_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT SUM(item_qty) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) });//std::cout << "total_cart_qty: " << total_qty << std::endl;
    // 10 items max
    int total_items = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM cart_item WHERE cart_id = $1", { std::to_string(cart_id) });//std::cout << "total_cart_items (rows): " << total_items << std::endl;
    return (total_items >= max_items || total_qty >= max_quantity);
#endif    
    return false;
}
////////////////////
bool neroshop::Cart::in_cart(int user_id, unsigned int item_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL) 
    // fetch id FROM cart WHERE user_id = $1
    int cart_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM cart WHERE user_id = $1", { std::to_string(user_id) });   
    // check if item is in cart
    bool item_in_cart = (DB::Postgres::get_singleton()->get_text_params("SELECT EXISTS (SELECT item_id FROM cart_item WHERE cart_id = $1 AND item_id = $2)", { std::to_string(cart_id), std::to_string(item_id) }) == "t") ? true : false;
    return item_in_cart;
#endif    
    return false;
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
