#include "user.hpp"

#include "cart.hpp"
#include "protocol/transport/client.hpp"
#include "database/database.hpp"
#include "tools/logger.hpp"

#include <fstream>

////////////////////
neroshop::User::User() : id(""), logged(false), account_type(UserAccountType::Guest), cart(nullptr), order_list({}), favorites_list({}) {
    cart = std::unique_ptr<Cart>(new Cart());
}
////////////////////
neroshop::User::~User()
{
    // clear private key
    private_key.clear();
    // destroy cart
    if(cart.get()) cart.reset();
    // clear orders
    order_list.clear(); // this should reset (delete) all orders
    // clear favorites
    favorites_list.clear(); // this should reset (delete) all favorites
#ifdef NEROSHOP_DEBUG
    std::cout << "user deleted\n";
#endif    
}
////////////////////
////////////////////
////////////////////
// buyers can only rate seller they have purchased from!!
void neroshop::User::rate_seller(const std::string& seller_id, int score, const std::string& comments, const std::string& signature) { // perfected 99.9%!!
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");    
    // seller_id cannot be 0 (0 = invalid id)
    if(seller_id.empty()) return;
    // score must be between 0 and 1
    if(score >= 1) score = 1;
    if(score <= 0) score = 0;
    /*int account_type_id = database->get_integer_params("SELECT account_type_id FROM users WHERE id = $1", { seller_id });//std::string account_type = db.get_column_text("users", "account_type", "id = " + seller_id);
    if(account_type_id != 2) {neroshop::print("This user (id: " + seller_id + ") is not a seller, so they cannot be rated", 2); return;}//if(String::lower(account_type) != "seller") {neroshop::print("You cannot rate a non-seller");return;}*/
    // Prevent seller from rating him/herself
    if(seller_id == get_id()) {
        neroshop::print("You cannot rate yourself", 2);
        return; // exit function
    }
    // To prevent duplicating seller_id that is has already been rated by this user_id (a user cannot rate the same seller twice, except update his or her score rating for a specific seller_id
    std::string rated_seller = database->get_text_params("SELECT seller_id FROM seller_ratings "
        "WHERE seller_id = $1 AND user_id = $2", { seller_id, get_id() });
	if(rated_seller == seller_id) { 
	    neroshop::print("You have previously rated this seller (id: " + seller_id + ")", 2);
	    database->execute_params("UPDATE seller_ratings SET score = $1 WHERE seller_id = $2 AND user_id = $3", { std::to_string(score), seller_id, get_id() });
	    database->execute_params("UPDATE seller_ratings SET comments = $1 WHERE seller_id = $2 AND user_id = $3", { comments, seller_id, get_id() });
	    database->execute_params("UPDATE seller_ratings SET signature = $1 WHERE seller_id = $2 AND user_id = $3", { signature, seller_id, get_id() });
	    neroshop::print("Your rating for seller (id: " + seller_id + ") has been updated to a score of " + ((score != 0) ? "\033[1;32m" : "\033[1;91m") + std::to_string(score) + "\033[0m");
	        
	    return; // exit function
	}
    // Insert initial values
    database->execute_params("INSERT INTO seller_ratings (seller_id, score, user_id, comments, signature) "
        "VALUES ($1, $2, $3, $4, $5)", { seller_id,
    std::to_string(score), get_id(), comments, signature });
    neroshop::print("You have rated seller (id: " + seller_id + ") with a score of " + ((score != 0) ? "\033[1;32m" : "\033[1;91m") + std::to_string(score) + "\033[0m");
    #ifdef NEROSHOP_DEBUG
    // Get number of seller ratings (for a specific seller_id)
    unsigned int total_seller_ratings = database->get_integer_params("SELECT COUNT(*) FROM seller_ratings WHERE seller_id = $1", { seller_id });
    std::cout << "total ratings for seller (id: " << seller_id << "): \033[1;93m" << total_seller_ratings << "\033[0m" << std::endl;
    // Get seller rating from user (0 = bad, 1 = good)
    unsigned int good_ratings = database->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { seller_id, std::to_string(1) });
    std::cout << "number of good ratings for seller (id: " << seller_id << "): \033[1;32m" << good_ratings << "\033[0m" << std::endl;
    unsigned int bad_ratings  = database->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { seller_id, std::to_string(0) });                
    std::cout << "number of bad ratings for seller  (id: " << seller_id << "): \033[1;91m" << bad_ratings << "\033[0m" << std::endl;
    // Calculate seller reputation
    double reputation = (good_ratings / static_cast<double>(total_seller_ratings)) * 100;
    std::cout << "reputation of seller (id: " << seller_id << "):\033[0;93m " << static_cast<int>(reputation) << "%\033[0m" << std::endl;
    #endif  
} 
// int seller_id = 2;
// user->rate_seller(seller_id, 1, "This seller rocks!");
// user->rate_seller(seller_id, 0, "This seller sucks!");
////////////////////
////////////////////
void neroshop::User::rate_item(const std::string& product_id, int stars, const std::string& comments, const std::string& signature) { // perfected 99%!!!
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // If item is not registered
    if(product_id.empty()) return; // exit function
    // star ratings must be between 1 and 5
    if(stars >= 5) stars = 5;
    if(stars <= 1) stars = 1;
    /*// Check if user has previously ordered this item
    bool purchased = false;
    for(int i = 0; i < get_order_count(); i++) {
        std::string ordered_item = database->get_text_params("SELECT product_id FROM order_item WHERE product_id = $1 AND order_id = $2", { product_id, std::to_string(get_order(i)->get_id()) });
        if(ordered_item == product_id) {
            //std::cout << "You previously ordered this item \033[0;35m(order_id: " << get_order(i)->get_id() << "\033[0m, contains \033[0;93mproduct_id: " << product_id << "\033[0m)" << std::endl;
            purchased = true; // set purchased to true since user has previously purchased this item
            break; // break from the for-loop so we don't stay stuck in loop forever
        }
    }
    if(!purchased) {neroshop::print("You must purchase this item (id: " + product_id + ") before rating it");  return;}*/
    // To prevent duplicating product_id that is has already been rated by this user_id (a user cannot rate (insert star ratings) for the same item twice, except update his or her star rating for a specific item)
    std::string rated_item = database->get_text_params("SELECT product_id FROM product_ratings " 
        "WHERE product_id = $1 AND user_id = $2", { product_id, get_id() });
	if(rated_item == product_id) { 
	    neroshop::print("You have previously rated this item (id: " + product_id + ")", 2);
	    // if user has previous rated this item, update the user's rating
	    database->execute_params("UPDATE product_ratings SET stars = $1 WHERE product_id = $2 AND user_id = $3", { std::to_string(stars), product_id, get_id() });
	    database->execute_params("UPDATE product_ratings SET comments = $1 WHERE product_id = $2 AND user_id = $3", { comments, product_id, get_id() });
	    database->execute_params("UPDATE product_ratings SET signature = $1 WHERE product_id = $2 AND user_id = $3", { signature, product_id, get_id() });
	    neroshop::print("Your star rating on item (id: " + product_id + ") has been updated to \033[1;33m" + std::to_string(stars) + "\033[1;37m stars\033[0m");
	        
	    return; // exit function
	}        
    // Insert initial values
    database->execute_params("INSERT INTO product_ratings (product_id, stars, user_id, comments, signature) "
        "VALUES ($1, $2, $3, $4, $5)", { product_id, std::to_string(stars),
    get_id(), comments, signature });
    neroshop::print("You have rated item (id: " + product_id + ") with " + ((stars >= 3) ? "\033[1;32m" : "\033[1;91m") + std::to_string(stars) + " \033[0mstars");
    #ifdef NEROSHOP_DEBUG
    // Get number of star ratings (for a specific product_id)
    unsigned int total_star_ratings = database->get_integer_params("SELECT COUNT(*) FROM product_ratings WHERE product_id = $1", { product_id });
    std::cout << "# star ratings for item (id: " << product_id << "): \033[1;33m" << total_star_ratings << "\033[0m" << std::endl;
    // Get number of 1, 2, 3, 4, and 5 star_ratings   
    int one_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id, std::to_string(1) });
    std::cout << "# of 1 star ratings for item (id: " << product_id << "):\033[0;93m " << one_star_count << "\033[0m" << std::endl;
    int two_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id, std::to_string(2) });
    std::cout << "# of 2 star ratings for item (id: " << product_id << "):\033[0;93m " << two_star_count << "\033[0m" << std::endl;
    int three_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id, std::to_string(3) }); 
    std::cout << "# of 3 star ratings for item (id: " << product_id << "):\033[0;93m " << three_star_count << "\033[0m" << std::endl;
    int four_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id, std::to_string(4) });
    std::cout << "# of 4 star ratings for item (id: " << product_id << "):\033[0;93m " << four_star_count << "\033[0m" << std::endl;
    int five_star_count = database->get_integer_params("SELECT COUNT(stars) FROM product_ratings WHERE product_id = $1 AND stars = $2", { product_id, std::to_string(5) });        
    std::cout << "# of 5 star ratings for item (id: " << product_id << "):\033[0;93m " << five_star_count << "\033[0m" << std::endl;
    // Now calculate average stars
    // 3 total star ratings:(5, 3, 4) // average = (5 + 3 + 4) / 3 = 4 stars        
    double average_stars = (
        (1 * static_cast<double>(one_star_count)) + 
        (2 * static_cast<double>(two_star_count)) + 
        (3 * static_cast<double>(three_star_count)) + 
        (4 * static_cast<double>(four_star_count)) + 
        (5 * static_cast<double>(five_star_count))) / total_star_ratings;
    std::cout << "calculated average stars for item (id: " << product_id << "):\033[1;33m " << average_stars << "\033[0m" << std::endl;
    // Test average at: https://calculator.academy/average-rating-calculator-star-rating/#f1p1|f2p0
    #endif
} 
// user->rate_item(ball.get_id(), 5, "Definitely not a Dragon ball!");
////////////////////
////////////////////
// account-related stuff here
////////////////////
void neroshop::User::convert() {
    if(is_guest()) return;
    if(is_seller()) { 
        neroshop::print("You are already a seller", 2); 
        return; 
    }
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ///database->execute("BEGIN;"); // not necessary unless doing multiple operations
    database->execute_params("UPDATE users SET account_type_id = $1 WHERE id = $2", { std::to_string(2), std::to_string(this->id) });
    neroshop::print("You have converted from a buyer to a seller", 3);    
    ////database->execute("COMMIT;");
    ////////////////////////////////    
#endif    
}
// if(user->is_buyer()) user->convert(); // convert buyer to seller
////////////////////
/*void neroshop::User::revert() {
    // convert user from seller to buyer
    UPDATE users SET account_type_id = 1 WHERE id = $1;
    // remove all items listed by this user
    DELETE FROM inventory WHERE seller_id = $1;
}
*/
////////////////////
void neroshop::User::delete_account() {
    if(!is_logged()) {neroshop::print("You are not logged in", 2);return;} // must be logged in to delete your account
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    ////database->execute("BEGIN;"); // not necessary unless doing multiple operations
    ////database->execute("SAVEPOINT before_account_deletion_savepoint;");//ROLLBACK TO before_account_deletion_savepoint;
    database->execute_params("DELETE FROM users WHERE id = $1", { this->id });
    neroshop::print("your account has been permanently deleted", 1);
    // send account to deleted accounts table (or graveyard >:})
    //if(!database->table_exists("deleted_users")) {
    //database->create_table("deleted_users");
    //database->add_column("deleted_users", "name", "text REFERENCES users(name)");//"text");
    //database->create_index("idx_deleted_users", "deleted_users", "name");
    //}
    //database->execute_params("INSERT INTO deleted_users (name) VALUES ($1)", { this->name });
    // reset user information and logout user
    set_id(0);
    name.clear();
    set_account_type(UserAccountType::Guest);
    set_logged(false); // logout here (will call on_logout callback, if logged is false)    
    // end transaction
    ////database->execute("COMMIT;");
} // username of deleted accounts should be reused or nah?
////////////////////
////////////////////
////////////////////
// cart-related stuff here
////////////////////
void neroshop::User::add_to_cart(const std::string& product_id, int quantity) {
    cart->add(this->id, product_id, quantity);
}
////////////////////
void neroshop::User::add_to_cart(const neroshop::Product& item, int quantity) {
    add_to_cart(item.get_id(), quantity);
}
////////////////////
void neroshop::User::remove_from_cart(const std::string& product_id, int quantity) {
    cart->remove(this->id, product_id, quantity);
}
////////////////////
void neroshop::User::remove_from_cart(const neroshop::Product& item, int quantity) {
    remove_from_cart(item.get_id(), quantity);
}
////////////////////
void neroshop::User::clear_cart() {
    cart->empty();
}
////////////////////
////////////////////
////////////////////
// order-related stuff here
////////////////////
void neroshop::User::create_order(const std::string& shipping_address) {//const {
    // name(first, last), address1(street, p.o box, company name, etc.), address2(apt number, suite, unit, building, floor, etc.) city, zip/postal_code, state/province/region country, optional(phone, email)
    try {
    std::shared_ptr<neroshop::Order> order(std::make_shared<neroshop::Order>());//(new neroshop::Order());
    order->create_order(*cart.get(), shipping_address); // we are using crypto, not debit/credit cards so no billing address is needed
    order_list.push_back(order); // whether an order fails or succeeds, store it regardless
    } catch(std::exception& e) {
        std::cout << e.what() << "\n";
    }
}
// cart->add(ball, 2);
// cart->add(ring);
// user->create_order(shipping_addr);
////////////////////
// put this in Buyer::on_login and Seller::on_login
// Guests orders are not saved to the main database
// orders are never deleted, their statuses just change: rejected, failure, delivered, etc.
void neroshop::User::load_orders() {
    /*DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("orders")) {db.close(); return;} // user probably has no order history
    // create orders based on user order_ids stored in orders
    // get last inserted order
    int last_order = db.get_column_integer("orders ORDER BY id DESC LIMIT 1", "*");//int orders_count = db.row_count("orders");
    int user_order_count = db.get_column_integer("orders", "COUNT(*)", "user_id = " + get_id());// get number of orders this user has made so far
    if(user_order_count < 1) neroshop::print("No orders found"); // for this user
    if(user_order_count > 0) 
    {   // if user has any orders, load them
        for(unsigned int i = 1; i <= last_order; i++) { // i is wrong because ids can start from 24, but i starts from 1
            unsigned int order_id = db.get_column_integer("orders", "id", "id = " + std::to_string(i) + " AND user_id = " + get_id());
            if(order_id == 0) continue; // skip 0's
            neroshop::Order * order = new neroshop::Order(order_id);
            order_list.push_back(order); // store orders for later use
            neroshop::print("Order (id: " + std::to_string(order->get_id()) + ") has been loaded");
        }
    }
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    //if(!database->table_exists("orders")) { return;} // user probably has no order history
    // first, check if user has any orders
    int user_order_count = database->get_integer_params("SELECT COUNT(*) FROM orders WHERE user_id = $1", { get_id() });// get number of orders this user has made so far
    if(user_order_count < 1) {neroshop::print("No order history found on your account");  return;} // for this user    
    // THIS IS FASTER THAN THE FORMER, SINCE IT DOES NOT LOOP THROUGH ALL THE ORDERS IN THE TABLE orders
    std::string command = "SELECT id FROM orders WHERE user_id = $1 ORDER BY id"; // sort by id; ASC order is the default (lowest-to-highest)
    std::vector<const char *> param_values = { get_id().c_str() };
    PGresult * result = PQexecParams(database->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    int rows = PQntuples(result);//if(rows < 1) {PQclear(result);  return;}    
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("User::load_orders(): You have no orders in your account", 2);        
        PQclear(result);//exit(1);
        return; // exit so we don't double free "result" or double close the database
    }
    for(int i = 0; i < rows; i++) {
        int order_id = std::stoi(PQgetvalue(result, i, 0));
        std::shared_ptr<neroshop::Order> order(std::make_shared<neroshop::Order>(order_id));//(new neroshop::Order(order_id));//neroshop::Order * order = new neroshop::Order(order_id);
        order_list.push_back(order); // store orders for later use        
        neroshop::print("Order (id: " + std::to_string(order->get_id()) + ") has been loaded");
    }    
    PQclear(result);
    ////////////////////////////////    
    
    ////////////////////////////////    
#endif    
}
////////////////////
////////////////////
////////////////////
// favorite-or-wishlist-related stuff
////////////////////
void neroshop::User::add_to_favorites(const std::string& product_id) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // check if item is already in favorites so that we do not add the same item more than once
    std::string item_name = database->get_text_params("SELECT name FROM item WHERE id = $1", { product_id });
    bool favorited = (database->get_text_params("SELECT EXISTS(SELECT product_ids FROM favorites WHERE $1 = ANY(product_ids) AND user_id = $2)", { product_id, std::to_string(this->id) }) == "t") ? true : false;
    if(favorited) { neroshop::print("\"" + item_name + "\" is already in your favorites", 2); return; /* exit function */}
    // add item to favorites
    database->execute_params("UPDATE favorites SET product_ids = array_append(product_ids, $1::integer) WHERE user_id = $2", { product_id, std::to_string(this->id) });
    // store in vector as well
    favorites_list.push_back(std::make_shared<neroshop::Product>(product_id));//(std::unique_ptr<neroshop::Product>(new neroshop::Product(product_id)));
    neroshop::print("\"" + item_name + "\" has been added to your favorites", 3);//if(std::find(favorites_list.begin(), favorites_list.end(), product_id) == favorites_list.end()) { favorites_list.push_back(product_id); neroshop::print("\"" + item_name + "\" has been added to your favorites", 3); }// this works for a favorite_list that stores integers (product_ids) rather than the item object itself
#endif    
}
////////////////////
void neroshop::User::add_to_favorites(const neroshop::Product& item) {
    add_to_favorites(item.get_id());
}
////////////////////
void neroshop::User::remove_from_favorites(const std::string& product_id) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // check if item has already been removed from favorites so that we don't have to remove it more than once
    std::string item_name = database->get_text_params("SELECT name FROM item WHERE id = $1", { product_id });
    bool favorited = (database->get_text_params("SELECT EXISTS(SELECT product_ids FROM favorites WHERE $1 = ANY(product_ids) AND user_id = $2)", { product_id, std::to_string(this->id) }) == "t") ? true : false;
    if(!favorited) return; // exit function ////{ neroshop::print("\"" + item_name + "\" was not found in your favorites list", 2); return; }
    // remove item from favorites
    database->execute_params("UPDATE favorites SET product_ids = array_remove(product_ids, $1::integer) WHERE user_id = $2", { product_id, std::to_string(this->id) });
    // remove from vector as well
    for(const auto & favorites : favorites_list) {
        if(favorites->get_id() != product_id) continue; // skip items whose ids do the match the product_id to be deleted
        auto it = std::find(favorites_list.begin(), favorites_list.end(), favorites);
        int item_index = it - favorites_list.begin();//std::cout << "favorites_list item index: " << item_index << std::endl;
        favorites_list.erase(favorites_list.begin() + item_index);
        if(std::find(favorites_list.begin(), favorites_list.end(), favorites) == favorites_list.end()) neroshop::print("\"" + item_name + "\" has been removed from your favorites", 1); // confirm that item has been removed from favorites_list
    }    
#endif        
}
////////////////////
void neroshop::User::remove_from_favorites(const neroshop::Product& item) {
    remove_from_favorites(item.get_id());
}
////////////////////
void neroshop::User::clear_favorites() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // first check if array is empty
    int is_empty = database->get_integer_params("SELECT COUNT(*) FROM favorites WHERE product_ids = '{}' AND user_id = $1", { std::to_string(this->id) });
    if(is_empty) return; // array is empty so that means there is nothing to delete, exit function
    // clear all items from favorites
    database->execute_params("UPDATE favorites SET product_ids = '{}' WHERE user_id = $1", { std::to_string(this->id) });
    // clear favorites from vector as well
    favorites_list.clear();
    if(favorites_list.empty()) neroshop::print("your favorites have been cleared");// confirm that favorites_list has been cleared
#endif        
}
////////////////////
void neroshop::User::load_favorites() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    std::string command = "SELECT unnest(product_ids) FROM favorites WHERE user_id = $1";
    std::vector<const char *> param_values = { std::to_string(this->id).c_str() };
    PGresult * result = PQexecParams(database->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    int rows = PQntuples(result);//if(rows < 1) { PQclear(result); return; }
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("User::load_favorites(): Your favorites list is empty", 2);        
        PQclear(result);//exit(1);
        return; // exit so that we don't double free "result"
    }
    for(int i = 0; i < rows; i++) {
        int product_id = std::stoi(PQgetvalue(result, i, 0));
        favorites_list.push_back(std::make_shared<neroshop::Product>(product_id));//(std::unique_ptr<neroshop::Product>(new neroshop::Product(product_id))); // store favorited_items for later use
        neroshop::print("Favorited item (id: " + product_id + ") has been loaded");
    }
    PQclear(result);
#endif    
}
////////////////////
////////////////////
////////////////////
// avatar-related stuff here
////////////////////
void neroshop::User::upload_avatar(const std::string& filename) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    //----------------------------
    //unsigned int user_id = 1;
    //----------------------------
    std::string image_file = filename;
    std::string image_name = image_file.substr(image_file.find_last_of("\\/") + 1);// get filename from path
    image_name = image_name.substr(0, image_name.find_last_of(".")); // remove ext
    std::string image_ext = image_file.substr(image_file.find_last_of(".") + 1);
    
    std::cout << "image name: " << image_name << std::endl;
    std::cout << "image file: " << image_file << std::endl;
    std::cout << "image extension: " << image_ext << std::endl;    
    ///////////////////////////
    // Todo: replace user id with monero_address
    //////////////////////////
    database->execute("BEGIN;");
    //////////////////////////
    // Todo: check if column exists first
    bool row_exists = database->get_integer_params("SELECT EXISTS(SELECT * FROM users WHERE monero_address = $1);", { this->id });
    if(!row_exists) {
        std::cout << "User with id " << this->id << " does not exist\n";
        database->execute("ROLLBACK;"); // abort transaction
        return;
    }
    //////////////////////////
    //static char lo_content[] = "Lorem ipsum dolor sit amet, fabulas conclusionemque ius ad.";//std::cout << "lo_content size: " << sizeof(lo_content) << std::endl;//60
    std::ifstream image_file_r(image_file, std::ios::binary); // std::ios::binary is the same as std::ifstream::binary
    if(!image_file_r.good()) {
        std::cout << NEROSHOP_TAG "failed to load " << image_file << std::endl; 
        database->execute("ROLLBACK;"); return;
    }
    image_file_r.seekg(0, std::ios::end); // std::ios::end is the same as image_file_r.end
    size_t size = static_cast<int>(image_file_r.tellg()); // in bytes
    // Limit avatar image size to 262144 bytes (256 kilobytes)
    const int max_bytes = 262144;
    double kilobytes = max_bytes / 1024.0;//std::cout << max_bytes << " bytes is equal to " << kilobytes << " kilobytes." << std::endl;
    // Todo: Database cannot scale to billions of users if I am storing blobs so I'll have to switch to text later
    if(size >= max_bytes) {
        neroshop::print("Avatar upload image cannot exceed " + std::to_string(kilobytes) + " KB", 1);
        database->execute("ROLLBACK;"); return;
    }
    image_file_r.seekg(0); // image_file_r.seekg(0, image_file_r.beg);
    std::vector<unsigned char> buffer(size); // or unsigned char buffer[size];// or unsigned char * buffer = new unsigned char[size];
    if(!image_file_r.read(reinterpret_cast<char *>(&buffer[0]), size)) {// read data as a block
        std::cout << NEROSHOP_TAG "error: only " << image_file_r.gcount() << " could be read";
        database->execute("ROLLBACK;"); // abort transaction
        return; // exit function
    }    
    ////for(auto content : buffer) std::cout << content << std::endl; // temp
    image_file_r.close();
    ///////////////////////////
    std::string command = "UPDATE users SET avatar = $1 WHERE monero_address = $2;";
    sqlite3_stmt * statement = nullptr;
    int result = sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        database->execute("ROLLBACK;"); return;// nullptr;
    }
    // Bind user-defined parameter arguments
    result = sqlite3_bind_blob(statement, 1, buffer.data(), size, SQLITE_STATIC);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_bind_blob: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(statement);
        database->execute("ROLLBACK;"); return;// nullptr;
    }  
    
    result = sqlite3_bind_text(statement, 2, this->id.c_str(), this->id.length(), SQLITE_STATIC);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(statement);
        database->execute("ROLLBACK;"); return;// nullptr;
    }  
    
    result = sqlite3_step(statement);
    if (result != SQLITE_DONE) {
        neroshop::print("sqlite3_step: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
    }
        
    sqlite3_finalize(statement);
    ///////////////////////////
    database->execute("COMMIT;");
    std::cout << "uploadImage succeeded\n";
}
////////////////////
bool neroshop::User::export_avatar() {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    //----------------------------
    // Check if avatar column exists first and that its not null
    bool has_avatar = database->get_integer_params("SELECT EXISTS(SELECT avatar FROM users WHERE monero_address = $1 AND avatar IS NOT NULL);", { this->id });
    if(!has_avatar) {
        neroshop::print("No avatar found", 2);
        return false;//database->execute("ROLLBACK;"); return false;
    }    
    //----------------------------
    // Check if default appdata path exists
    std::string data_path = NEROSHOP_DEFAULT_DATABASE_PATH;
    if(!std::filesystem::is_directory(data_path)) { 
        neroshop::print("directory \"" + data_path + "\" not found", 1); 
        return false; 
    }
    // Create cache folder within the default appdata path
    std::string cache_folder = data_path + "/cache";
    if(!std::filesystem::is_directory(cache_folder)) { 
        neroshop::print("Creating directory \"" + cache_folder + "\" (^_^) ...", 2);
        if(!std::filesystem::create_directories(cache_folder)) {
            neroshop::print("Failed to create folder \"" + cache_folder + "\" (ᵕ人ᵕ)!", 1);
            return false;
        }
        neroshop::print("\033[1;97;49mcreated path \"" + cache_folder + "\""); // bold bright white
    }
    // Generate a name for the avatar (to save to a file)
    std::string image_filename = cache_folder + "/avatar_" + this->id;
    // Check if image already exists in cache so that we do not export the same image more than once
    if(std::filesystem::is_regular_file(image_filename)) {
        neroshop::print(image_filename + " already exists", 2);
        return true;
    }
    // Open file for writing
    std::ofstream image_file_w(image_filename.c_str(), std::ios::binary);    
    if(!image_file_w.is_open()) {
        neroshop::print("Failed to open " + image_filename, 1); 
        return false;
    }
    //----------------------------
    // Get the avatar's image data (blob) from the database
    std::string command = "SELECT avatar FROM users WHERE monero_address = $1";
    sqlite3_stmt * statement = nullptr;
    int bytes = 0;
    // Prepare (compile) the statement
    int result = sqlite3_prepare_v2(database->get_handle(), command.c_str(), -1, &statement, nullptr);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_prepare_v2: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        return false;
    }
    // Bind text to user id (1st arg)
    result = sqlite3_bind_text(statement, 1, this->id.c_str(), this->id.length(), SQLITE_STATIC);
    if(result != SQLITE_OK) {
        neroshop::print("sqlite3_bind_text: " + std::string(sqlite3_errmsg(database->get_handle())), 1);
        sqlite3_finalize(statement);
        return false;
    }      
    // Execute the statement
    result = sqlite3_step(statement);
    // Get image size in bytes
    if(result == SQLITE_ROW) {
        bytes = sqlite3_column_bytes(statement, 0);//std::cout << "bytes: " << bytes << std::endl;
    }    
    // Write (export) to file
    image_file_w.write(reinterpret_cast<const char*>(static_cast<const unsigned char *>(sqlite3_column_blob(statement, 0))), bytes);//reinterpret_cast<unsigned char*>(const_cast<const void *>(sqlite3_column_blob(statement, 0)));
    // close file
    image_file_w.close();
    // Finalize the statement    
    sqlite3_finalize(statement);    
    //printf("Autocommit: %d\n", sqlite3_get_autocommit(database->get_handle()));
    //----------------------------    
    neroshop::print("exported \"" + image_filename + "\" to \"" + cache_folder + "\"", 3);
    return true;
}
////////////////////
void neroshop::User::delete_avatar() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // begin transaction (required when dealing with large objects)
    database->execute("BEGIN;");
    /////////////////////////////////////////////////
    // get avatar oid (large object)
    Oid avatar_oid = database->get_integer_params("SELECT data FROM avatars WHERE user_id = $1;", { std::to_string(this->id) });//std::cout << avatar_oid << std::endl;
    if(avatar_oid == InvalidOid) { database->execute("ROLLBACK;"); return; } // ABORT; is the same as ROLLBACK;
    // delete large object
    int lo_result = lo_unlink(database->get_handle(), avatar_oid);
    if(lo_result == -1) {
        neroshop::print(POSTGRESQL_TAG "lo_unlink failed: " + std::string(PQerrorMessage(database->get_handle())), 1);
        database->execute("ROLLBACK;"); // abort transaction
        return;// false;
    }       
    neroshop::print("your avatar (oid: " + std::to_string(avatar_oid) + ") has been deleted", 1);    
    /////////////////////////////////////////////////
    // end transaction
    database->execute("COMMIT;");
    //return true;
#endif    
}
////////////////////
////////////////////
void neroshop::User::set_public_key(const std::string& public_key) {
    // TODO: validate public key before setting it
    this->public_key = public_key;
}
////////////////////
void neroshop::User::set_private_key(const std::string& private_key) {
    this->private_key = private_key;
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
//void neroshop::User::set_id(unsigned int id) {
//    this->id = id;
//}
////////////////////
void neroshop::User::set_id(const std::string& id) {
    this->id = id;
}
////////////////////
void neroshop::User::set_name(const std::string& name) {
    this->name = name;
}
////////////////////
void neroshop::User::set_account_type(UserAccountType account_type) {
    this->account_type = account_type;
}
////////////////////
void neroshop::User::set_logged(bool logged) { // protected function, so only derived classes can use this
    this->logged = logged;
    if(!logged) logout(); // call on_logout() (callback)
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
//unsigned int neroshop::User::get_id() const {
//    return id;
//}
////////////////////
std::string neroshop::User::get_id() const {
    return id;
}
////////////////////
std::string neroshop::User::get_name() const {
    return name;
}
////////////////////
UserAccountType neroshop::User::get_account_type() const {
    return account_type;
}
////////////////////
std::string neroshop::User::get_account_type_string() const {
    switch(this->account_type) {
        case UserAccountType::Guest: return "Guest"; break;
        case UserAccountType::Buyer: return "Buyer"; break;
        case UserAccountType::Seller: return "Seller"; break;
        default: return ""; break;
    }
}
////////////////////
std::string neroshop::User::get_public_key() const {
    return public_key;
}

std::string neroshop::User::get_private_key() const {
    return private_key;
}
////////////////////
////////////////////
neroshop::Cart * neroshop::User::get_cart() const {
    return cart.get();
}
////////////////////
////////////////////
neroshop::Order * neroshop::User::get_order(unsigned int index) const {
    if(index > (order_list.size() - 1)) throw std::out_of_range("neroshop::User::get_order(): attempt to access invalid index");
    return order_list[index].get();
}
////////////////////
unsigned int neroshop::User::get_order_count() const {
    return order_list.size();
}
////////////////////
std::vector<neroshop::Order *> neroshop::User::get_order_list() const {
    std::vector<neroshop::Order *> orders = {};
    for(const auto & order : order_list) {//for(int o = 0; o < order_list.size(); o++) {
        orders.push_back(order.get());//(order_list[o].get());
    }
    return orders;
}
////////////////////
////////////////////
neroshop::Product * neroshop::User::get_favorite(unsigned int index) const {
    if(index > (favorites_list.size() - 1)) throw std::out_of_range("neroshop::User::get_favorites(): attempt to access invalid index");
    return favorites_list[index].get();
}
////////////////////
unsigned int neroshop::User::get_favorites_count() const {
    return favorites_list.size();
}
////////////////////
std::vector<neroshop::Product *> neroshop::User::get_favorites_list() const {
    std::vector<neroshop::Product *> favorites = {};
    for(const auto & item : favorites_list) {//for(int f = 0; f < favorites_list.size(); f++) {
        favorites.push_back(item.get());//(favorites_list[f].get());
    }
    return favorites;
}
////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::User::is_guest() const {
    if(is_logged()) return false;
    return true; // guests (buyers) are not required to register // guests are buyers by default, except their data is not stored
}
////////////////////
bool neroshop::User::is_buyer() const// buyer and guests are not required to register, only sellers
{
    ////if(id < 1) return false;
    /*DB::Sqlite3 db("neroshop.db");
    int account_type_id = db.get_column_integer("users", "account_type_id", "id = " + std::to_string(this->id));//std::string account_type = db.get_column_text("users", "account_type", "id = " + std::to_string(this->id));
    if(account_type_id <= 0) return false;//if(String::lower(account_type) != "buyer" || account_type.empty()) return false;
    if(account_type_id != 1) return false; // 1 = buyer
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    int account_type_id = database->get_integer_params("SELECT account_type_id FROM users WHERE id = $1::int", { std::to_string(this->id) });
    if(account_type_id != 1) {return false;} // 1 = buyer //neroshop::print("User " + name + " is a buyer", 4); // only print when there's an error   
    
    ////////////////////////////////
    return true;
#endif
    return false;    
}
////////////////////
bool neroshop::User::is_seller() const
{
    ////if(id < 1) return false;//if(String::lower(this->name) == "guest") return false; // reserved name "Guest" for guests only
    /*DB::Sqlite3 db("neroshop.db");
    int account_type_id = db.get_column_integer("users", "account_type_id", "id = " + std::to_string(this->id));//std::string account_type = db.get_column_text("users", "account_type", "id = " + std::to_string(this->id));
    if(account_type_id <= 0) return false;//if(account_type.empty() || String::lower(account_type) != "seller") return false;
    if(account_type_id != 2) return false; // 2 = seller
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    int account_type_id = database->get_integer_params("SELECT account_type_id FROM users WHERE id = $1::int", { std::to_string(this->id) });
    if(account_type_id != 2) {return false;} // 2 = seller //neroshop::print("User " + name + " is a seller", 4); // only print when there's an error
    
    ////////////////////////////////
    return true;
#endif
    return false;    
}
////////////////////
bool neroshop::User::is_online() const // a user is not created until they are logged so this function can only be called when a user is logged // guests can also use this function so its a bad idea to check if user is logged
{
    return Client::get_main_client()->is_connected();// && is_logged()); // user must be both connected to the network and logged in
}
////////////////////
bool neroshop::User::is_registered() const {
    /*DB::Sqlite3 db("neroshop.db");
	// if table Users does not exist, that means no accounts are registered
	if(!db.table_exists("users")) return false; // if table Users does not exist, no accounts are registered
	// confirm that this user's id is in the db (to further prove that they are registered)
	int id = db.get_column_integer("users", "id", "id = " + get_id());
	if(id <= 0) return false;
	db.close();*/ // always remember to close db when done :)
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
	// if table Users does not exist, that means no accounts are registered
	if(!database->table_exists("users")) {
	    ////neroshop::print("Failed to retrieve data (Database is ether missing or corrupted or table users simply does not exist)", 1);
	    
	    return false; // failed to retrieve username
	}
	// confirm that this user's id is in the db (to further prove that they are registered)
	int user_id = database->get_integer_params("SELECT id FROM users WHERE id = $1", { get_id() });
	if(id < 1) {
	    ////neroshop::print("You are not a registered user", 1);
	    
	    return false;    
    }
    //neroshop::print("You are a registered user", 3); // only print when there is an error
    
    ////////////////////////////////	
    return true;
#endif
    return false;    
}
////////////////////
bool neroshop::User::is_registered(const std::string& name) { // no need to login to prove user is registered, just need to check the db
    // an empty username is ALWAYS invalid
    if(name.empty()) return false;
    ////////////////////////////////
    // sqlite
    ////////////////////////////////    
    /*DB::Sqlite3 db("neroshop.db");
	// if table Users does not exist, that means no accounts are registered
	if(!db.table_exists("users")) return false;
	// confirm that this user's name is in the db (to prove that they are registered)
	std::string user = db.get_column_text("users", "name", "name = " + DB::Sqlite3::to_sql_string(String::lower(name)));
	if(user.empty()) return false;
	if(user != String::lower(name)) return false;
	db.close();*/ // always remember to close db when done :)
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
	// confirm that this user's name is in the db (to prove that they are registered)
	std::string user = database->get_text_params("SELECT name FROM users WHERE name = $1", { String::lower(name) });
	// if table users does not exist, that means no accounts are registered
	if(!database->table_exists("users")) {
	    neroshop::print("Failed to retrieve data (Database is ether missing or corrupted or table users simply does not exist)", 1);
	    
	    return false; // failed to retrieve username
	}
	if(user.empty()) {
	    neroshop::print("No user with such name found: \033[1;97m" + name + "\033[0m", 1); // Found no user with such name
	    
	    return false; // no user with such name
    }
    //neroshop::print(user + " is a registered user", 3);
    
    ////////////////////////////////	
    return true;
#endif
    return false;    
}
////////////////////
bool neroshop::User::is_logged() const
{
    return logged;
}
////////////////////
bool neroshop::User::has_email() const {
    if(is_guest()) return false;
    ////////////////////////////////
    // sqlite
    ////////////////////////////////
    /*DB::Sqlite3 db("neroshop.db");
    std::string email_hash = db.get_column_text("users", "email", "id = " + get_id());
    if(email_hash.empty()) return false;
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    std::string email = database->get_text_params("SELECT opt_email FROM users WHERE id = $1::int", { get_id() });
    if(email.empty()) {
        neroshop::print("no email found on account", 2);
        
        return false;
    }
    if(!email.empty()) neroshop::print("email found: " + email, 3);
    
    ////////////////////////////////
    return true;
#endif
    return false;    
}
////////////////////
bool neroshop::User::has_avatar() const {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // If id is zero (this means the user does not exist)
    if(this->id.empty()) return false;  
    // Check if avatar column exists first and that its not null
    bool is_valid_avatar = database->get_integer_params("SELECT EXISTS(SELECT avatar FROM users WHERE monero_address = $1 AND avatar IS NOT NULL);", { this->id });
    if(!is_valid_avatar) {
        neroshop::print("No avatar found", 2);
        return false;//database->execute("ROLLBACK;"); return false;
    }    
    return true;
}
////////////////////
////////////////////
bool neroshop::User::has_purchased(const std::string& product_id) { // for registered users only//if(!is_logged()) { neroshop::print("You are not logged in", 2); return false; }
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // check if user has previously ordered this item (we have the order_ids which we'll need to figure out which order_items belong to each order_id)
    std::vector<unsigned int> purchases_list = {};
    for(const auto & orders : order_list) { 
        int order_item = database->get_integer_params("SELECT product_id FROM order_item WHERE product_id = $1 AND order_id = $2", { product_id, std::to_string(orders->get_id()) });
        // to-do: check if order was not cancelled or refunded
        if(order_item < 1) continue; // skip invalid ids
        if(order_item == product_id) {//std::cout << "user_order_ids: " << orders->get_id() << std::endl;
            // store purchased product_ids if not yet stored
            if(std::find(purchases_list.begin(), purchases_list.end(), product_id) == purchases_list.end()) {
                purchases_list.push_back(order_item);
            }
        }
    }
    return (std::find(purchases_list.begin(), purchases_list.end(), product_id) != purchases_list.end());
#endif
    return false;    
}
////////////////////
bool neroshop::User::has_purchased(const neroshop::Product& item) {
    return has_purchased(item.get_id());
}
////////////////////
bool neroshop::User::has_favorited(const std::string& product_id) {
    // since we loaded the favorites into memory when the app launched, we should be able to access the pre-loaded favorites and any newly added favorites in the current session without performing any database queries/operations
    for(const auto & favorites : favorites_list) {
        // if any favorites_list items' ids matches "product_id" then return true
        if(favorites->get_id() == product_id) return true;
    }
    return false;////return (std::find(favorites_list.begin(), favorites_list.end(), product_id) != favorites_list.end()); // this is good for when storing favorites as integers (product_ids)
}
////////////////////
bool neroshop::User::has_favorited(const neroshop::Product& item) {
    return has_favorited(item.get_id());
}
////////////////////
////////////////////
////////////////////
// callbacks
////////////////////
//User * neroshop::User::on_login(const std::string& username) {return nullptr;} // this function does nothing
////////////////////
void neroshop::User::on_order_received() {} // for sellers to implement // this function does nothing
////////////////////
void neroshop::User::logout() {
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
    neroshop::print("You have logged out");
}
////////////////////
////////////////////
////////////////////
