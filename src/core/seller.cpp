#include "seller.hpp"

#include "cart.hpp"
#include "tools/logger.hpp"
#include "tools/uuid.hpp"
#include "database/database.hpp"
#include "product.hpp"
#include "wallet/wallet.hpp"
#include "listing.hpp"
#include "product.hpp"
#include "protocol/transport/client.hpp"
#include "protocol/p2p/serializer.hpp"
#include "category.hpp"
#include "tools/base64.hpp"
#include "crypto/rsa.hpp"
#include "tools/timestamp.hpp"

#include <cmath> // floor
#include <random>

namespace neroshop {

Seller::Seller()
{}
////////////////////
////////////////////
Seller::Seller(const std::string& name) : Seller() {
    set_name(name);
}
////////////////////
////////////////////
Seller::~Seller() {
    // clear customer orders
    customer_order_list.clear(); // will reset (delete) all customer orders
#ifdef NEROSHOP_DEBUG    
    std::cout << "seller deleted\n";
#endif    
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
std::string Seller::list_item(
    const std::string& name,
    const std::string& description,
    const std::vector<ProductAttribute>& attributes,
    const std::string& product_code,
    int category_id,
    const std::vector<int>& subcategory_ids,
    const std::vector<std::string>& tags,
    const std::vector<Image>& images,
    
    unsigned int quantity, 
    double price, 
    const std::string& currency, 
    const std::string& condition, 
    const std::string& location,
    unsigned int quantity_per_order
) const
{
    // Transition from Sqlite to DHT:
    Client * client = Client::get_main_client();
    // Create product object
    const std::string listing_id = neroshop::uuid::generate();//std::cout << "listing id: " << listing_id << "\n";
    const std::string product_id = listing_id;
    Product product {
        product_id, name, description, attributes, 
        product_code, static_cast<unsigned int>(category_id), subcategory_ids, tags,
        images
    };
    // Create listing object
    const std::string seller_id = get_id();//std::cout << "seller_id: " << seller_id << "\n";

    std::string created_at = neroshop::timestamp::get_current_utc_timestamp();//std::cout << "created_at: " << created_at << "\n";
    
    std::string signature = wallet->sign_message(listing_id, monero_message_signature_type::SIGN_WITH_SPEND_KEY);//std::cout << "signature: " << signature << "\n\n";
    
    #ifdef NEROSHOP_DEBUG0
    auto result = wallet->verify_message(listing_id, signature);
    std::cout << "\033[1mverified: " << (result == 1 ? "\033[32mpass" : "\033[91mfail") << "\033[0m\n";
    assert(result == true);
    #endif
    
    Listing listing { listing_id, product, seller_id,
        quantity, price, currency,
        condition, location, created_at, signature, quantity_per_order
    };//listing.print_listing();
    
    auto data = Serializer::serialize(listing);
    std::string key = data.first;
    std::string value = data.second;//std::cout << "key: " << data.first << "\nvalue: " << data.second << "\n";
    
    // Send put request to neighboring nodes (and your node too JIC)
    std::string response;
    client->put(key, value, response);
    std::cout << "Received response (put): " << response << "\n";
    
    // Return listing key
    return key;
}
////////////////////
void Seller::delist_item(const std::string& listing_key) {
    // Transition from Sqlite to DHT:
    Client * client = Client::get_main_client();
    // TODO: remove product from table cart_item, table images, table products, and table product_ratings as well
    // Get the value of the corresponding key from the DHT
    std::string response;
    client->get(listing_key, response); // TODO: error handling
    std::cout << "Received response (get): " << response << "\n";
    // Parse the response
    nlohmann::json json = nlohmann::json::parse(response);
    if(json.contains("error")) {
        neroshop::print("delist_item: key is lost or missing from DHT", 1);
        return; // Key is lost or missing from DHT, return
    }    
    
    const auto& response_obj = json["response"];
    assert(response_obj.is_object());
    if (response_obj.contains("value") && response_obj["value"].is_string()) {
        const auto& value = response_obj["value"].get<std::string>();
        nlohmann::json value_obj = nlohmann::json::parse(value);
        assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
        std::string metadata = value_obj["metadata"].get<std::string>();
        if (metadata != "listing") { std::cerr << "Invalid metadata. \"listing\" expected, got \"" << metadata << "\" instead\n"; return; }
        // Skip if already delisted
        int quantity = value_obj["quantity"];
        if(quantity <= 0) { return; }
        // Verify ownership
        std::string seller_id = value_obj["seller_id"].get<std::string>();
        if(seller_id != wallet->get_primary_address()) {
            neroshop::print("delist_item: you cannot delist this since you are not the listing's creator", 1);
            return;
        }
        // Verify with the signature
        std::string listing_id = value_obj["id"].get<std::string>();
        std::string old_signature = value_obj["signature"].get<std::string>();
        bool self_verified = wallet->verify_message(listing_id, old_signature);
        if(!self_verified) { neroshop::print("Data verification failed.", 1); return; }
        // Might be a good idea to set the stock quantity to zero beforehand
        value_obj["quantity"] = 0;
        // Not possible to completely remove data from DHT unless it originally had an expiration date
        // So the least we could do is set the quantity to zero
        //value_obj["expiration_date"] = neroshop::timestamp::get_utc_timestamp_after_duration(24, "hour");
        // Re-sign to reflect the modification
        std::string signature = wallet->sign_message(listing_id, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
        value_obj["signature"] = signature;
        value_obj["last_updated"] = neroshop::timestamp::get_current_utc_timestamp();
        // Send set request containing the updated value with the same key as before
        std::string modified_value = value_obj.dump();
        std::string response;
        client->set(listing_key, modified_value, response);
        std::cout << "Received response (set): " << response << "\n";
    }
}
////////////////////
////////////////////
////////////////////
// the moment the seller logs in, they should be notified that they have a pending transaction from a customer
// and should respond swiftly
// if seller accepts the order, then an address will be generated from seller's wallet and sent to the customer
// if seller rejects the order, their stock_qty is increased by the failed order's qty
void Seller::load_customer_orders() {
    /*db::Sqlite3 db("neroshop.db");
    ///////////
    if(!db.table_exists("order_item")) return; // seller has probably never received an order from a customer before
    // check for orders made by customers
    // get last inserted order item
    int last_order_item = db.get_column_integer("order_item ORDER BY id DESC LIMIT 1", "*");
    // get all order_items
    int customer_order_item_count = db.get_column_integer("order_item", "COUNT(*)", "seller_id = " + get_id());
    //std::cout << "number of items that customers have ordered from you: " << customer_order_item_count << std::endl;
    if(customer_order_item_count < 1) neroshop::print("No buyer has ordered an item from you yet");
    if(customer_order_item_count > 0) {
        for(unsigned int i = 1; i <= last_order_item; i++) {
            //if order_item's order_id is duplicated, then it means there are multiple unique items in the order
            unsigned int order_product_id = db.get_column_integer("order_item", "id", "id = " + std::to_string(i) + " AND seller_id = " + get_id());
            if(order_product_id == 0) continue; // skip 0's
            // get order_id of the order_item
            unsigned int order_id = db.get_column_integer("order_item", "order_id", "id = " + std::to_string(i) + " AND seller_id = " + get_id());//if(order_id == 0) continue; // skip 0's
            // store order_ids if not already stored
            if(std::find(customer_order_list.begin(), customer_order_list.end(), order_id) == customer_order_list.end()) {
                customer_order_list.push_back(order_id); //Order * order = new Order(order_id);//customer_order_list.push_back(order);
                neroshop::print("Customer order (id: " + std::to_string(order_id) + ") has been loaded");
            }
            // get items in the order_item table
            const std::string& product_id = db.get_column_integer("order_item", "product_id", "id = " + std::to_string(i) + " AND seller_id = " + get_id());
            unsigned int item_qty = db.get_column_integer("order_item", "item_qty", "id = " + std::to_string(i) + " AND seller_id = " + get_id());
            Product item(product_id); // item obj will die at the end of this scope
            std::cout << "You've received an order (id: " << order_id << ") from a customer " 
            << "containing items: " << item.get_name() << " (id: " << product_id << ", qty: " << item_qty << ")" << std::endl;
        }
    }
    ///////////
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////////////////
    // get number of order_items to be purchased by customers from this particular seller
    int seller_customer_order_item_count = database->get_integer_params("SELECT COUNT(*) FROM order_item WHERE seller_id = $1", { get_id() });
    if(seller_customer_order_item_count < 1) {neroshop::print("No buyer has ordered an item from you yet"); return;}    
    // load customer orders
    std::string command = "SELECT order_id FROM order_item WHERE seller_id = $1 ORDER BY order_id";
    std::vector<const char *> param_values = { get_id().c_str() };
    PGresult * result = PQexecParams(database->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Seller::load_customer_orders(): No customer orders found", 2);        
        PQclear(result);
        //exit(1);
        return; // exit so we don't double free "result"
    }
    int rows = PQntuples(result);
    for(int i = 0; i < rows; i++) {
        int order_id = std::stoi(PQgetvalue(result, i, 0));
        // store order_ids if not already stored
        if(std::find(customer_order_list.begin(), customer_order_list.end(), order_id) == customer_order_list.end()) {
            customer_order_list.push_back(order_id); //Order * order = new Order(order_id);//customer_order_list.push_back(order);
            neroshop::print("Customer order (id: " + std::to_string(order_id) + ") has been loaded");
        }       
        /*#ifdef NEROSHOP_DEBUG0
            // get items in the order_item table
            const std::string& product_id = database->get_integer_params("SELECT product_id FROM order_item WHERE id = $1 AND seller_id = $2", { std::to_string(i), get_id() });
            unsigned int item_qty = database->get_integer_params("SELECT item_qty FROM order_item WHERE id = $1 AND seller_id = $2", { std::to_string(i), get_id() });
            Product item(product_id); // item obj will die at the end of this scope
            std::cout << "You've received an order (id: " << order_id << ") from a customer " 
            << "containing items: " << item.get_name() << " (id: " << product_id << ", qty: " << item_qty << ")" << std::endl;
        #endif    */        
    } //database->get_integer_params("SELECT product_id FROM order_item WHERE id = $1 AND seller_id = $2", { std::to_string(i), get_id() });
    ////////////////////////////////    
    
    ////////////////////////////////    
#endif    
}
////////////////////
// THIS FUNCTION WILL BE LISTENING FOR ANY NEW (PENDING) ORDERS AT ALL TIMES
void Seller::update_customer_orders() { // this function is faster (I think) than load_customer_orders()
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////////////////
    //customer_order_list.clear(); // No need to clear customer_orders since it only inserts unique order_ids, so it will not take any duplicates
    std::string command = "SELECT order_id FROM order_item WHERE seller_id = $1";
    std::vector<const char *> param_values = { get_id().c_str() };
    PGresult * result = PQexecParams(database->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Seller::update_customer_orders(): No customer orders found", 2);        
        PQclear(result);
        //exit(1);
        return; // exit so we don't double free "result" or double close the database
    }
    int rows = PQntuples(result);
    for(int i = 0; i < rows; i++) {
        int customer_order_id = std::stoi(PQgetvalue(result, i, 0));
        // store order_ids if not already stored (does NOT store duplicates)
        if(std::find(customer_order_list.begin(), customer_order_list.end(), customer_order_id) == customer_order_list.end()) {
            // check if order is a pending order
            bool is_pending = database->get_integer_params("SELECT id FROM orders WHERE id = $1 AND status = $2", { std::to_string(customer_order_id), "Pending" });
            if(is_pending) {
                std::cout << "You have received a new customer order (status: PENDING)" << std::endl; // for terminal
                neroshop::Message::get_first()->set_text("You have received a new customer order (status: PENDING)");//\n                                  Do you wish to proceed with this order?");
                // maybe send the seller an email as well? :O
                // Do you wish to process this order?
                // [accept] [decline]
                // box text
                //int vertical_padding = 20; // top and bottom padding
                neroshop::Message::get_first()->get_label(0)->set_alignment("none");
                neroshop::Message::get_first()->get_label(0)->set_relative_position((neroshop::Message::get_first()->get_width() / 2) - (neroshop::Message::get_first()->get_label(0)->get_string().length() * 10/*neroshop::Message::get_first()->get_label(0)->get_width()*/ / 2), ((neroshop::Message::get_first()->get_height() - 10/*neroshop::Message::get_first()->get_label(0)->get_height()*/) / 2) - 20); // 50=label_rel_y_pos
                // box buttons - CRASH SITE
                Button * button0 = neroshop::Message::get_first()->get_button(0);
                Button * button1 = neroshop::Message::get_first()->get_button(1);
                button0->set_text("Accept");//Accept//Respond//View//Reply//Answer//Return
                button1->set_text("Decline");//Decline//Refuse//Ignore//Reject//Forget//Mark as read
                button0->set_width(100);
                button1->set_width(100);
                // the height of the msgbox almost NEVER changes, only its width
                int button_gap = 10; // the space between button0 and button1
                button0->set_relative_position((neroshop::Message::get_first()->get_width() / 2) - (button0->get_width() / 2) - ((button1->get_width() + button_gap) / 2), neroshop::Message::get_first()->get_height() - button0->get_height() - 20);//20 = bottom_padding
                button1->set_relative_position(button0->get_relative_x() + button0->get_width() + button_gap, button0->get_relative_y());
                button0->set_color(0, 107, 61, 1.0);//(99, 151, 84, 1.0);//
                button1->set_color(214, 31, 31, 1.0);//(224, 60, 50, 1.0);//
                button0->show();
                button1->show();
                // Supply a subaddress or generate a unique subaddress from your wallet (for receiving funds from the customer)
                // BUT if a seller supplies an address, then he/she will not receive the notification that they've received a deposit of x amount of xmr into their wallet
                //[supply address] [generate]
                // if no, then seller must supply a unique subaddress
                ////if(!monero_utils::is_valid_address(random_subaddress, monero_network_type::STAGENET)) {
                //    neroshop::print(random_subaddress + " is not a valid address", 1);
                //}             
                // if yes then a unique subaddress will be generated from your wallet for receiving funds from the customer
                // if user chooses to generate a unique subaddress:
                if(has_wallet_synced()) {
                    std::string subaddress;
                    on_order_received(subaddress);
                    neroshop::print("generated unique subaddress: " + subaddress);
                    // add the address to the seller's address book so they know which order the address belongs
                    ////wallet->address_book_add(subaddress, "For customer order with id: " + std::to_string(customer_order_id));
                }
                // if not connected to daemon or remote node, print "You cannot generate unless you connect to a node"
                // if no then you can retrieve your stock back
                neroshop::Message::get_first()->show();
            }
            // store order
            customer_order_list.push_back(customer_order_id);
        }
    }
    PQclear(result); // free result
    ////////////////////////////////
#endif    
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// setters - item and inventory-related stuff
////////////////////
void Seller::set_stock_quantity(const std::string& listing_key, int quantity) {
    // Transition from Sqlite to DHT:
    Client * client = Client::get_main_client();

    // Get the value of the corresponding key from the DHT
    std::string response;
    client->get(listing_key, response); // TODO: error handling
    std::cout << "Received response (get): " << response << "\n";
    // Parse the response
    nlohmann::json json = nlohmann::json::parse(response);
    if(json.contains("error")) {
        neroshop::print("set_stock_quantity: key is lost or missing from DHT", 1);
        return; // Key is lost or missing from DHT, return 
    }    
    
    const auto& response_obj = json["response"];
    assert(response_obj.is_object());
    if (response_obj.contains("value") && response_obj["value"].is_string()) {
        const auto& value = response_obj["value"].get<std::string>();
        nlohmann::json value_obj = nlohmann::json::parse(value);
        assert(value_obj.is_object());//std::cout << value_obj.dump(4) << "\n";
        std::string metadata = value_obj["metadata"].get<std::string>();
        if (metadata != "listing") { std::cerr << "Invalid metadata. \"listing\" expected, got \"" << metadata << "\" instead\n"; return; }
        // Verify ownership of the data (listing)
        std::string seller_id = value_obj["seller_id"].get<std::string>();
        if(seller_id != wallet->get_primary_address()) {
            neroshop::print("set_stock_quantity: you cannot modify this listing since you are not the listing's creator", 1);
            return;
        }
        // Verify the signature
        std::string listing_id = value_obj["id"].get<std::string>();
        std::string old_signature = value_obj["signature"].get<std::string>();
        bool self_verified = wallet->verify_message(listing_id, old_signature);
        if(!self_verified) { neroshop::print("Data verification failed.", 1); return; }
        // Finally, modify the quantity
        value_obj["quantity"] = quantity;
        // Re-sign to reflect the modification
        std::string signature = wallet->sign_message(listing_id, monero_message_signature_type::SIGN_WITH_SPEND_KEY);
        value_obj["signature"] = signature;
        // Add a last_modified or last_updated field so nodes can compare dates and choose the most recent listing
        value_obj["last_updated"] = neroshop::timestamp::get_current_utc_timestamp();
        // Send set request containing the updated value with the same key as before
        std::string modified_value = value_obj.dump();
        std::string response;
        client->set(listing_key, modified_value, response);
        std::cout << "Received response (set): " << response << "\n";
    }
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
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// getters - seller rating system
////////////////////
unsigned int Seller::get_good_ratings() const {
    /*db::Sqlite3 db("neroshop.db");
    if(db.table_exists("seller_ratings")) {
        unsigned int good_ratings_count = db.get_column_integer("seller_ratings", "COUNT(score)", "seller_id = " + get_id() + " AND score = " + std::to_string(1));
        return good_ratings_count;
    }
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
	if(!database->table_exists("seller_ratings")) { return 0;}
    unsigned int good_ratings_count = database->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { get_id(), std::to_string(1) });
	
	return good_ratings_count;
	////////////////////////////////
#endif	
    return 0;
}
unsigned int Seller::get_bad_ratings() const {
    /*db::Sqlite3 db("neroshop.db");
    if(db.table_exists("seller_ratings")) {
        unsigned int bad_ratings_count = db.get_column_integer("seller_ratings", "COUNT(score)", "seller_id = " + get_id() + " AND score = " + std::to_string(0));
        return bad_ratings_count;
    }
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
	if(!database->table_exists("seller_ratings")) { return 0;}
    unsigned int bad_ratings_count = database->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { get_id(), std::to_string(0) });    	
	
	return bad_ratings_count;
	////////////////////////////////    
#endif
    return 0;	
}
////////////////////
unsigned int Seller::get_ratings_count() const {
    /*db::Sqlite3 db("neroshop.db");
    if(db.table_exists("seller_ratings")) {
        unsigned int ratings_count = db.get_column_integer("seller_ratings", "COUNT(*)", "seller_id = " + get_id());
        return ratings_count;
    }
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
    if(!database->table_exists("seller_ratings")) { return 0;}
    unsigned int ratings_count = database->get_integer_params("SELECT COUNT(*) FROM seller_ratings WHERE seller_id = $1", { get_id() });    
	
    return ratings_count;
    ////////////////////////////////
#endif    
    return 0;
}
////////////////////
unsigned int Seller::get_total_ratings() const {
    return get_ratings_count();
}
////////////////////
unsigned int Seller::get_reputation() const {
    /*neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    // Get seller reputation as percentage
    unsigned int ratings_count = database->get_integer_params("SELECT COUNT(*) FROM seller_ratings WHERE seller_id = $1", { get_id() });
    if(ratings_count == 0) return 0; // seller has not yet been rated so his or her reputation will be 0%
    // Get seller's good (positive) ratings
    unsigned int good_ratings = database->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { get_id(), std::to_string(1) });
    // Calculate seller reputation
    double reputation = (good_ratings / static_cast<double>(ratings_count)) * 100;
    return static_cast<int>(reputation); // convert reputation to an integer (for easier readability)*/
    return 0;
}
////////////////////
std::vector<unsigned int> Seller::get_top_rated_sellers(unsigned int limit) {
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // get n seller_ids with the most positive (good) ratings
    // ISSUE: both seller_4 and seller_1 have the same number of 1_score_values but seller_1 has the highest reputation and it places seller_4 first [solved - by using reputation in addition]
    std::string command = "SELECT users.id FROM users JOIN seller_ratings ON users.id = seller_ratings.seller_id WHERE score = 1 GROUP BY users.id ORDER BY COUNT(score) DESC LIMIT $1;";
    std::vector<const char *> param_values = { std::to_string(limit).c_str() };
    PGresult * result = PQexecParams(database->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Seller::get_top_rated_sellers(): No sellers found", 2);        
        PQclear(result);
        return {}; // exit so that we don't double free "result"
    }
    int rows = PQntuples(result);
    std::vector<unsigned int> top_rated_seller_ids = {};
    for(int i = 0; i < rows; i++) {
        int seller_id = std::stoi(PQgetvalue(result, i, 0));
        // calculate the reputation of each seller_id
        unsigned int ratings_count = database->get_integer_params("SELECT COUNT(*) FROM seller_ratings WHERE seller_id = $1", { std::to_string(seller_id) });
        if(ratings_count == 0) continue; // seller has not yet been rated so his or her reputation will be 0%. Skip this seller
        int good_ratings = database->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { std::to_string(seller_id), std::to_string(1) });
        double reputation = (good_ratings / static_cast<double>(ratings_count)) * 100;        
        // store the top rated seller_ids (only if they have a certain high reputation)
        if(reputation >= 90) { // a reputation of 90 and above makes you a top rated seller (maybe I will reduce it to 85 just to be more fair or nah)
            top_rated_seller_ids.push_back(seller_id);
            if(std::find(top_rated_seller_ids.begin(), top_rated_seller_ids.end(), seller_id) != top_rated_seller_ids.end()) std::cout << "top rated sellers: " << seller_id << " (reputation: " << static_cast<int>(reputation) << ")" << std::endl;
        }
    }
    PQclear(result); // free result
    //--------------------------------------------------------------
    // get n seller_ids with the least negative (bad) reviews
    // amongst the sellers with the most positive (good) reviews
    // "SELECT users.id, COUNT(score) AS bad_ratings_count FROM users JOIN seller_ratings ON users.id = seller_ratings.seller_id WHERE score = 0 GROUP BY users.id ORDER BY COUNT(score) ASC;"
    // ...
    //--------------------------------------------------------------    
    return top_rated_seller_ids;
#endif
    return {};    
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// getters - order-related stuff
////////////////////
unsigned int Seller::get_customer_order(unsigned int index) const {
    if(customer_order_list.empty()) return 0;//return nullptr;
    if(index > (customer_order_list.size() - 1)) throw std::out_of_range("Seller::get_customer_order(): attempt to access invalid index");
    return customer_order_list[index];
}
////////////////////
unsigned int Seller::get_customer_order_count() const {
    return customer_order_list.size();
}
////////////////////
std::vector<int> Seller::get_pending_customer_orders() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    std::vector<int> pending_order_list;
    ////////////////////////////////
    //database->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////////////////
    // update customer_order_list (by adding any new orders or orders that have not yet been added)
    update_customer_orders();
    ////////////////////////////////
    // now lets get all the pending orders from the UPDATED customer_order_list  
    std::string pending_order_msg = "Pending orders awaiting seller approval: (ids: ";
    for(int i = 0; i < customer_order_list.size(); i++) {
         int pending_orders = database->get_integer_params("SELECT id FROM orders WHERE id = $1 AND status = $2", { std::to_string(customer_order_list[i]), "Pending" });
         if(pending_orders != 0) {
             pending_order_list.push_back(customer_order_list[i]);
             // gather ids of pending_orders
             pending_order_msg += std::to_string(pending_orders) + "; ";
         }
    }
    pending_order_msg.append(")");
    pending_order_msg = neroshop::string::remove_last_of(pending_order_msg, "; ");
#ifdef NEROSHOP_DEBUG
    std::cout << pending_order_msg << std::endl;
#endif
    ////////////////////////////////
    // notify seller of a pending customer order
    /*if(pending_order_list.size() > 0) {
        Message::get_first()->set_text("You have " + std::to_string(pending_order_list.size()) + " pending customer orders");//\n                                  Do you wish to proceed with this order?");
        //neroshop::Message message("You have " + std::to_string(pending_order_list.size()) + " pending customer orders");
             // Do you wish to process this order?
             // [accept] [decline]
             // Supply a subaddress or generate a unique subaddress from your wallet (for receiving funds from the customer)
             // if yes then a unique subaddress will be generated from your wallet for receiving funds from the customer
             //[supply address] [generate]
             // if not connected to daemon or remote node, print "You cannot generate unless you connect to a node"
             // if no then you can retrieve your stock back
        Message::get_first()->show();//message.show();//Message::get_first()->show();
        //message.draw();
    }*/
    ////////////////////////////////
    
    ////////////////////////////////
    return pending_order_list;
#endif
    return {};    
}
////////////////////
////////////////////
////////////////////
// getters - sales and statistics-related stuff
////////////////////
unsigned int Seller::get_products_count() const {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");
    
    int products_listed = database->get_integer_params("SELECT COUNT(DISTINCT key) FROM mappings WHERE search_term = $1 AND content = 'listing';", { get_id() });
    return products_listed;
}
////////////////////
unsigned int Seller::get_sales_count() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // should item not be considered sold until the order is done processing or nah ?
	int items_sold = database->get_integer_params("SELECT SUM(item_qty) FROM order_item WHERE seller_id = $1;", { get_id() });
	return items_sold;
#endif
    return 0;	
}
////////////////////
unsigned int Seller::get_units_sold(const std::string& product_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    int units_sold = database->get_integer_params("SELECT SUM(item_qty) FROM order_item WHERE product_id = $1 AND seller_id = $2", { product_id, get_id() });
    return units_sold;
#endif
    return 0;    
}
////////////////////
unsigned int Seller::get_units_sold(const neroshop::Product& item) const {
    return get_units_sold(item.get_id());
}
////////////////////
double Seller::get_sales_profit() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    double profit_from_sales = database->get_real_params("SELECT SUM(item_price * item_qty) FROM order_item WHERE seller_id = $1;", { get_id() });//neroshop::print("The overall profit made from all sales combined is: $" + std::to_string(profit_from_sales), 3);
    return profit_from_sales;
#endif    
    return 0.0;
}
////////////////////
double Seller::get_profits_made(const std::string& product_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    double item_profits = database->get_real_params("SELECT SUM(item_price * item_qty) FROM order_item WHERE product_id = $1 AND seller_id = $2;", { product_id, get_id() });//std::string item_name = database->get_text_params("SELECT name FROM item WHERE id = $1", { product_id });neroshop::print("The overall profit made from \"" + item_name + "\" is: $" + std::to_string(item_profits), 3);
    return item_profits;
#endif    
    return 0.0;
}
////////////////////
double Seller::get_profits_made(const neroshop::Product& item) const {
    return get_profits_made(item.get_id());
}
////////////////////
unsigned int Seller::get_product_id_with_most_sales() const { // this function is preferred over the "_by_mode" version as it provides the most accurate best-selling product_id result
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // get the item with the biggest quantity sold (returns multiple results but I've limited it to 1)
    int item_with_biggest_qty = database->get_integer_params("SELECT product_id FROM order_item WHERE seller_id = $1 GROUP BY product_id ORDER BY SUM(item_qty) DESC LIMIT 1;", { get_id() }); // from the biggest to smallest sum of item_qty
#ifdef NEROSHOP_DEBUG
    std::string item_name = database->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_with_biggest_qty) });
    neroshop::print("\"" + item_name + "\" is your best-selling item with a sale of " + std::to_string(get_units_sold(item_with_biggest_qty)) + " units", 3);
#endif    
    return item_with_biggest_qty;
#endif
    return 0;    
}
////////////////////
unsigned int Seller::get_product_id_with_most_orders() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // get the item with the most occurences in all orders - if two items are the most occuring then it will select the lowest product_id of the two (unless I add DESC)
    int item_with_most_occurrences = database->get_integer_params("SELECT MODE() WITHIN GROUP (ORDER BY product_id) FROM order_item WHERE seller_id = $1;", { get_id() });
#ifdef NEROSHOP_DEBUG
    std::string item_name = database->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_with_most_occurrences) });
    int times_occured = database->get_integer_params("SELECT COUNT(*) FROM order_item WHERE product_id = $1 AND seller_id = $2;", { std::to_string(item_with_most_occurrences), get_id() });
    neroshop::print("\"" + item_name + "\" is your most ordered item occuring a total of " + std::to_string(times_occured) + " times in all orders", 2);
#endif    
    return item_with_most_occurrences;
#endif
    return 0;    
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// boolean
////////////////////
bool Seller::has_listed(const std::string& product_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
	bool listed = (database->get_text_params("SELECT EXISTS(SELECT product_id FROM inventory WHERE product_id = $1 AND seller_id = $2);", { product_id, get_id() }) == "t") ? true : false;
	return listed;
#endif
    return false;	
}
////////////////////
bool Seller::has_listed(const neroshop::Product& item) const {
    return has_listed(item.get_id());
}
////////////////////
bool Seller::has_stock(const std::string& product_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    bool in_stock = (database->get_text_params("SELECT EXISTS(SELECT product_id FROM inventory WHERE product_id = $1 AND seller_id = $2 AND stock_qty > 0);", { product_id, get_id() }) == "t") ? true : false;
    return in_stock;
#endif
    return false;    
}
////////////////////
bool Seller::has_stock(const neroshop::Product& item) const {
    return has_stock(item.get_id());
}
////////////////////
////////////////////
////////////////////
// callbacks
////////////////////
neroshop::User * Seller::on_login(const neroshop::Wallet& wallet) { // assumes user data already exists in database
    std::string monero_primary_address = wallet.get_primary_address();
    if(!wallet.is_valid_address(monero_primary_address)) {
        neroshop::print("Invalid monero address");
        return nullptr;
    }
    // create a new user (seller)
    neroshop::User * user = new Seller();
    // set user properties retrieved from database
    dynamic_cast<Seller *>(user)->set_logged(true); // protected, so can only be accessed by child class obj    
    dynamic_cast<Seller *>(user)->set_id(monero_primary_address);
    dynamic_cast<Seller *>(user)->set_wallet(wallet);
    dynamic_cast<Seller *>(user)->set_account_type(UserAccountType::Seller);
    //-------------------------------
    /*// load orders
    dynamic_cast<Seller *>(user)->load_orders();*/
    // load wishlists
    dynamic_cast<Seller *>(user)->load_favorites();    
    /*// load customer_orders
    static_cast<Seller *>(user)->load_customer_orders();*/
    // Load cart (into memory)
    user->get_cart()->load(user->get_id());
    return user;
}
////////////////////
void Seller::on_order_received(std::string& subaddress) {
    if(!wallet.get()) throw std::runtime_error("wallet has not been initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    // TODO: check if order type is a direct pay/no escrow before generating a new subaddress
    // if wallet is not properly synced with the daemon, you can only generate used addresses
    // unless wallet is synced to a daemon, you will not be able to generate any unique addresses
    if(!wallet->get_monero_wallet()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon"); // Indicates if the wallet is synced with the daemon.
    // generate 10 new subaddress after each order (just to be sure there are enough unused subaddresses to choose from)
    for(int i = 0; i < 10; i++) wallet->address_new();
    // get a list of all unused subaddresses
    std::vector<monero::monero_subaddress> unused_subaddress_list = wallet->get_addresses_unused(0);
    // now pick from the list of unused subaddresses (random)
	std::random_device rd; // Generating random numbers with C++11's random requires an engine and a distribution.
    std::mt19937 mt(rd()); // This is an engine based on the Mersenne Twister 19937 (64 bits):
    std::uniform_real_distribution<double> dist(0, unused_subaddress_list.size() - 1);
    subaddress = unused_subaddress_list[static_cast<int>(dist(mt))].m_address.get();
    // copy random subaddress
    // USED SUBADDRESS IS NOT REMOVED FROM Wallet::address_unused() UNTIL THE SECOND CONFIRMATION (OUTPUT RECEIVED ...) 
#ifdef NEROSHOP_DEBUG0
    std::cout << std::endl << "subaddress (random): " << subaddress << "\n";
#endif
    // also, generate a qrcode too
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
}
