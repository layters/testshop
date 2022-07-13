#include "../include/seller.hpp"

neroshop::Seller::Seller() : wallet(nullptr)
{}
////////////////////
////////////////////
neroshop::Seller::Seller(const std::string& name) : Seller() {
    set_name(name);
}
////////////////////
////////////////////
neroshop::Seller::~Seller() {
    // clear customer orders
    customer_order_list.clear(); // will reset (delete) all customer orders
    // destroy wallet
    if(wallet.get()) wallet.reset();
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
void neroshop::Seller::list_item(unsigned int item_id, unsigned int stock_qty, double sales_price, std::string currency, double discount, unsigned int discounted_items, unsigned int discount_times, std::string discount_expiry, std::string condition)
{
#if defined(NEROSHOP_USE_POSTGRESQL)
    // seller must be logged in
    if(!is_logged()) {neroshop::print("You must be logged in to list an item", 2); return;}
    // user must be an actual seller, not a buyer
    if(!is_seller()) {neroshop::print("Must be a seller to list an item (id: " + std::to_string(item_id) + ")", 2); return;}
    // a seller can create an item and then register it to the database
    // but if the item is not registered then it cannot be listed
    if(item_id < 1) {NEROSHOP_TAG_OUT std::cout << "\033[0;91m" << "This item is not registered (invalid Item id)" << "\033[0m" << std::endl; return;}
    // make sure currency is supported
    if(!neroshop::Converter::is_supported_currency(currency)) {neroshop::print(currency + " is not a supported currency", 2); return;}
    ////////////////////////////////
    // sqlite
    ////////////////////////////////    
    /*// store item in database
    DB::Sqlite3 db("neroshop.db");
	//db.execute("PRAGMA journal_mode = WAL;"); // this may reduce the incidence of SQLITE_BUSY errors (such as database being locked) // https://www.sqlite.org/pragma.html#pragma_journal_mode
    // check if item is already in db
	// create Inventory table if it does not yet exist
	if(!db.table_exists("inventory")) { // if(!db.table_exists(""))
	    db.table("inventory"); // inventory_id will be auto generated (primary key)
	    db.column("inventory", "ADD", "item_id", "INTEGER");
	    db.column("inventory", "ADD", "seller_id", "INTEGER"); // store_id or seller_id or vendor_id
	    db.column("inventory", "ADD", "stock_qty", "INTEGER"); //db.column("inventory", "ADD", "stock_available", "TEXT"); // or in_stock
	    db.column("inventory", "ADD", "seller_price", "REAL"); //db.index("idx_item_ids", "inventory", "item_id"); actually, don't make item_id unique as multiple sellers could be selling the same item //db.execute("CREATE UNIQUE INDEX idx_seller_ids ON Inventory (seller_id);"); // sellers can have multiple items so seller_id should not be unique
	    db.column("inventory", "ADD", "currency", "TEXT"); // seller's currency of choice, which will be converted to xmr
	    db.column("inventory", "ADD", "seller_discount", "REAL"); // seller_discount per discounted_items
	    db.column("inventory", "ADD", "discount_qty", "INTEGER");
	    db.column("inventory", "ADD", "condition", "TEXT"); // seller_condition for each item
	}
    // to prevent duplicating item_id that is being sold by the same seller_id (a seller cannot list the same item twice, except change the stock amount)
    int item_id = db.get_column_integer("inventory", "item_id", 
        "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(get_id()));
	if(item_id == item_id) { neroshop::print("\033[1;33mYou have already listed this item (id: " + std::to_string(item_id) + ")\033[0m");return;}
	// say you have 50 of an item, for every 2 of the same item, you get $0.50 off
	//double total_discount_calc = (50 / 2) * 0.50;
	double total_discount = (stock_qty_or_num_items_to_apply_discount / discounted_items) * discount; // stock_qty would be replace with item_qty in this case
	// if discounted_items is 2 but you have 3 of the item, it will return $0.75, but we need to avoid that and make it $0.50
	// if qty is not a multiple of "discounted_items"
	// if there is a remainder then reduce the total discount
	//if((stock_qty % discounted_items) == 1) total_discount = total_discount - discount;
#ifdef NEROSHOP_DEBUG0
	std::cout << "\033[1;37m" << "for every " << discounted_items << " " << item.get_name() << "s, you get " << neroshop::Converter::get_currency_symbol(currency) << discount << " off (since you have x" << stock_qty << ", total discount is: " << neroshop::Converter::get_currency_symbol(currency) << total_discount << ")\033[0m" << std::endl;//" of an item"
#endif	
	// insert item in inventory
	db.insert("inventory", "item_id, seller_id, stock_qty, seller_price, currency, seller_discount, discount_qty, condition", 
	    std::to_string(item_id) + ", " + std::to_string(get_id()) + ", " + std::to_string(stock_qty) + ", " + std::to_string(sales_price) + ", " + DB::Sqlite3::to_sql_string(String::lower(currency)) + ", " + std::to_string(discount) + ", " + std::to_string(discounted_items) + ", " + DB::Sqlite3::to_sql_string(condition));
	NEROSHOP_TAG_OUT std::cout << "\033[1;37m" << item.get_name() << " (id: " << item_id << ", stock_qty: " << stock_qty << ") has been listed by seller \033[1;34m" << get_name() << " (id: " << get_id() << ")" << "\033[0m" << std::endl; // price per unit (of an item)
	db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    // apparently money is bad for currency, float as well so we gotta use numeric (or decimal)
    // https://www.postgresql.org/docs/current/datatype-numeric.html
    // numeric(3,2) will be able to store max 9.99 3-2 = 1
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    // create table: inventory
    if(!DB::Postgres::get_singleton()->table_exists("inventory")) { // if(!db.table_exists(""))
	    DB::Postgres::get_singleton()->create_table("inventory"); // inventory_id will be auto generated (primary key)
	    DB::Postgres::get_singleton()->add_column("inventory", "item_id", "integer REFERENCES item(id) ON DELETE CASCADE"); // if an item is deleted, it no longer exists so CASCADE will remove said item from the inventory as well // CASCADE isn't working when I drop table item? :O
	    DB::Postgres::get_singleton()->add_column("inventory", "seller_id", "integer REFERENCES users(id) ON DELETE CASCADE"); // store_id or seller_id or vendor_id // if seller is deleted, so will the seller's listing (there's no inventory without the seller)
	    DB::Postgres::get_singleton()->add_column("inventory", "stock_qty", "integer"); //db.column("inventory", "ADD", "stock_available", "TEXT"); // or in_stock
	    DB::Postgres::get_singleton()->add_column("inventory", "seller_price", "numeric(20, 12)");//"decimal(12,2)"); // price_per_item_unit //db.index("idx_item_ids", "inventory", "item_id"); actually, don't make item_id unique as multiple sellers could be selling the same item //db.execute("CREATE UNIQUE INDEX idx_seller_ids ON Inventory (seller_id);"); // sellers can have multiple items so seller_id should not be unique
	    DB::Postgres::get_singleton()->add_column("inventory", "currency", "text"); // seller's currency of choice, which will be converted to xmr
	    DB::Postgres::get_singleton()->add_column("inventory", "seller_discount", "numeric(20, 12)");//"real"); // 20 - 12 = 8 total digits (on left side / whole number side) // 12 decimals // seller_discount per discounted_items // or discount_per_x_items // money should not be used when dealing with multiple currencies, instead use numeric as suggested by postgresql.org // force 2 units precision for fiat currencies: 
	    DB::Postgres::get_singleton()->add_column("inventory", "discount_qty", "integer"); // number of items that the discount will apply to // or discounted_items // for every x item, you get a discount (seller_discount)
	    DB::Postgres::get_singleton()->add_column("inventory", "discount_times", "integer"); // number of times the discount can be used (in a single order)
	    DB::Postgres::get_singleton()->add_column("inventory", "discount_expiry", "timestamptz DEFAULT NULL"); // will be in UTC format // date the discount expires
	    DB::Postgres::get_singleton()->add_column("inventory", "condition", "text"); // seller_condition for each item
	}        
    // to prevent duplicating item_id that is being sold by the same seller_id (a seller cannot list the same item twice, except change the stock amount)
    int listed_item = DB::Postgres::get_singleton()->get_integer_params("SELECT item_id FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(get_id()) });
	if(listed_item == item_id) { 
	    neroshop::print("\033[1;33mYou have already listed this item (id: " + std::to_string(item_id) + ")\033[0m"); 
	    ////DB::Postgres::get_singleton()->execute("ROLLBACK;");
	    
	    return;
	}	    
    // begin transaction - this is not necessary since it is just a single operation
    ////DB::Postgres::get_singleton()->execute("BEGIN;");
    // create a restore point - no need for this
    ////DB::Postgres::get_singleton()->execute("SAVEPOINT before_seller_item_listing_savepoint;");
	// convert localtime to universal time
	//std::cout << "discount_exp_date to UTC: " << DB::Postgres::get_singleton()->localtimestamp_to_utc(discount_expiry) << std::endl;
	// SELECT TO_TIMESTAMP('2021 12 10', 'YYYY-MM-DD HH24:MI:SS');
	// SELECT TO_TIMESTAMP('2021-12-10', 'YYYY-MM-DD HH24:MI:SS');
	// SELECT TO_CHAR(now(), 'YYYY-MM-DD HH24:MI:SS') ;
	// insert item in inventory
	DB::Postgres::get_singleton()->execute_params("INSERT INTO inventory (item_id, seller_id, stock_qty, seller_price, currency, seller_discount, discount_qty, discount_times, discount_expiry, condition) "
	    "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, TO_TIMESTAMP($9,'YYYY-MM-DD HH24:MI:SS'), $10)", { std::to_string(item_id), std::to_string(get_id()), std::to_string(stock_qty), std::to_string(sales_price)/* price per unit (of an item)*/,
	    String::lower(currency), std::to_string(discount), std::to_string(discounted_items), std::to_string(discount_times), discount_expiry, condition });
	// end transaction
	////DB::Postgres::get_singleton()->execute("COMMIT;");
	//
	std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
	NEROSHOP_TAG_OUT std::cout << "\033[1;37m" << item_name << " (id: " << item_id << ", stock_qty: " << stock_qty << ") has been listed by seller \033[1;34m" << get_name() << " (id: " << get_id() << ")" << "\033[0m" << std::endl;
    ////////////////////////////////	
#endif    
}
////////////////////
void neroshop::Seller::list_item(const neroshop::Item& item, unsigned int stock_qty, double sales_price, std::string currency, double discount, unsigned int discounted_items, unsigned int discount_times, std::string discount_expiry, std::string condition) { // ex. 5% off 10 balls
    list_item(item.get_id(), stock_qty, sales_price, currency, discount, discounted_items, discount_times, discount_expiry, condition);
}
// static_cast<Seller *>(user)->list_item(ball, 50, 8.50, "usd", 0.50, 2, "new"); // $0.50 cents off every 2 balls
////////////////////
void neroshop::Seller::delist_item(unsigned int item_id) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    DB::Postgres::get_singleton()->execute_params("DELETE FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(get_id()) }); // update item stock to 0 beforehand or nah?
#endif    
}
////////////////////
void neroshop::Seller::delist_item(const neroshop::Item& item) {
    delist_item(item.get_id());
}
////////////////////
////////////////////
////////////////////
// the moment the seller logs in, they should be notified that they have a pending transaction from a customer
// and should respond swiftly
// if seller accepts the order, then an address will be generated from seller's wallet and sent to the customer
// if seller rejects the order, their stock_qty is increased by the failed order's qty
void neroshop::Seller::load_customer_orders() {
    /*DB::Sqlite3 db("neroshop.db");
    ///////////
    if(!db.table_exists("order_item")) return; // seller has probably never received an order from a customer before
    // check for orders made by customers
    // get last inserted order item
    int last_order_item = db.get_column_integer("order_item ORDER BY id DESC LIMIT 1", "*");
    // get all order_items
    int customer_order_item_count = db.get_column_integer("order_item", "COUNT(*)", "seller_id = " + std::to_string(get_id()));
    //std::cout << "number of items that customers have ordered from you: " << customer_order_item_count << std::endl;
    if(customer_order_item_count < 1) neroshop::print("No buyer has ordered an item from you yet");
    if(customer_order_item_count > 0) {
        for(unsigned int i = 1; i <= last_order_item; i++) {
            //if order_item's order_id is duplicated, then it means there are multiple unique items in the order
            unsigned int order_item_id = db.get_column_integer("order_item", "id", "id = " + std::to_string(i) + " AND seller_id = " + std::to_string(get_id()));
            if(order_item_id == 0) continue; // skip 0's
            // get order_id of the order_item
            unsigned int order_id = db.get_column_integer("order_item", "order_id", "id = " + std::to_string(i) + " AND seller_id = " + std::to_string(get_id()));//if(order_id == 0) continue; // skip 0's
            // store order_ids if not already stored
            if(std::find(customer_order_list.begin(), customer_order_list.end(), order_id) == customer_order_list.end()) {
                customer_order_list.push_back(order_id); //Order * order = new Order(order_id);//customer_order_list.push_back(order);
                neroshop::print("Customer order (id: " + std::to_string(order_id) + ") has been loaded");
            }
            // get items in the order_item table
            unsigned int item_id = db.get_column_integer("order_item", "item_id", "id = " + std::to_string(i) + " AND seller_id = " + std::to_string(get_id()));
            unsigned int item_qty = db.get_column_integer("order_item", "item_qty", "id = " + std::to_string(i) + " AND seller_id = " + std::to_string(get_id()));
            Item item(item_id); // item obj will die at the end of this scope
            std::cout << "You've received an order (id: " << order_id << ") from a customer " 
            << "containing items: " << item.get_name() << " (id: " << item_id << ", qty: " << item_qty << ")" << std::endl;
        }
    }
    ///////////
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////////////////
    // get number of order_items to be purchased by customers from this particular seller
    int seller_customer_order_item_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM order_item WHERE seller_id = $1", { std::to_string(get_id()) });
    if(seller_customer_order_item_count < 1) {neroshop::print("No buyer has ordered an item from you yet"); return;}    
    // load customer orders
    std::string command = "SELECT order_id FROM order_item WHERE seller_id = $1 ORDER BY order_id";
    std::vector<const char *> param_values = { std::to_string(get_id()).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
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
            unsigned int item_id = DB::Postgres::get_singleton()->get_integer_params("SELECT item_id FROM order_item WHERE id = $1 AND seller_id = $2", { std::to_string(i), std::to_string(get_id()) });
            unsigned int item_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT item_qty FROM order_item WHERE id = $1 AND seller_id = $2", { std::to_string(i), std::to_string(get_id()) });
            Item item(item_id); // item obj will die at the end of this scope
            std::cout << "You've received an order (id: " << order_id << ") from a customer " 
            << "containing items: " << item.get_name() << " (id: " << item_id << ", qty: " << item_qty << ")" << std::endl;
        #endif    */        
    } //DB::Postgres::get_singleton()->get_integer_params("SELECT item_id FROM order_item WHERE id = $1 AND seller_id = $2", { std::to_string(i), std::to_string(get_id()) });
    ////////////////////////////////    
    
    ////////////////////////////////    
#endif    
}
////////////////////
// THIS FUNCTION WILL BE LISTENING FOR ANY NEW (PENDING) ORDERS AT ALL TIMES
void neroshop::Seller::update_customer_orders() { // this function is faster (I think) than load_customer_orders()
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////////////////
    //customer_order_list.clear(); // No need to clear customer_orders since it only inserts unique order_ids, so it will not take any duplicates
    std::string command = "SELECT order_id FROM order_item WHERE seller_id = $1";
    std::vector<const char *> param_values = { std::to_string(get_id()).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
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
            bool is_pending = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM orders WHERE id = $1 AND status = $2", { std::to_string(customer_order_id), "Pending" });
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
void neroshop::Seller::set_stock_quantity(unsigned int item_id, unsigned int stock_qty) {
    // seller must be logged in
    if(!is_logged()) {NEROSHOP_TAG_OUT std::cout << "\033[0;91m" << "You must be logged in to set stock" << "\033[0m" << std::endl; return;}
    // user must be an actual seller, not a buyer
    if(!is_seller()) {neroshop::print("Must be a seller to set stock (id: " + std::to_string(item_id) + ")", 2); return;}    
    // a seller can create an item and then register it to the database
    if(item_id <= 0) {NEROSHOP_TAG_OUT std::cout << "\033[0;91m" << "Could not set stock_qty (invalid Item id)" << "\033[0m" << std::endl; return;}
    /*// update stock_qty in database
    DB::Sqlite3 db("neroshop.db");
	//db.execute("PRAGMA journal_mode = WAL;"); // this may reduce the incidence of SQLITE_BUSY errors (such as database being locked)
	if(db.table_exists("inventory"))
	    db.update("inventory", "stock_qty", std::to_string(stock_qty), "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(get_id()));
	db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest"); 
    DB::Postgres::get_singleton()->execute_params("UPDATE inventory SET stock_qty = $1 WHERE item_id = $2 AND seller_id = $3", { std::to_string(stock_qty), std::to_string(item_id), std::to_string(get_id()) });
    std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
    neroshop::print("\"" + item_name + "\"'s stock has been updated", 3);
    
    ////////////////////////////////
#endif    
}
////////////////////
void neroshop::Seller::set_stock_quantity(const neroshop::Item& item, unsigned int stock_qty) {
    set_stock_quantity(item.get_id(), stock_qty);
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// setters - wallet-related stuff
////////////////////
void neroshop::Seller::set_wallet(const neroshop::Wallet& wallet) {
    std::unique_ptr<neroshop::Wallet> seller_wallet(&const_cast<neroshop::Wallet&>(wallet));
    this->wallet = std::move(seller_wallet); // unique pointers cannot be copied, but can only be moved // "std::unique_ptr::release()" is a similar function but "std::move()" is better of the two
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
// getters - seller rating system
////////////////////
unsigned int neroshop::Seller::get_good_ratings() const {
    /*DB::Sqlite3 db("neroshop.db");
    if(db.table_exists("seller_ratings")) {
        unsigned int good_ratings_count = db.get_column_integer("seller_ratings", "COUNT(score)", "seller_id = " + std::to_string(get_id()) + " AND score = " + std::to_string(1));
        return good_ratings_count;
    }
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
	if(!DB::Postgres::get_singleton()->table_exists("seller_ratings")) { return 0;}
    unsigned int good_ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { std::to_string(get_id()), std::to_string(1) });
	
	return good_ratings_count;
	////////////////////////////////
#endif	
    return 0;
}
unsigned int neroshop::Seller::get_bad_ratings() const {
    /*DB::Sqlite3 db("neroshop.db");
    if(db.table_exists("seller_ratings")) {
        unsigned int bad_ratings_count = db.get_column_integer("seller_ratings", "COUNT(score)", "seller_id = " + std::to_string(get_id()) + " AND score = " + std::to_string(0));
        return bad_ratings_count;
    }
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
	if(!DB::Postgres::get_singleton()->table_exists("seller_ratings")) { return 0;}
    unsigned int bad_ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { std::to_string(get_id()), std::to_string(0) });    	
	
	return bad_ratings_count;
	////////////////////////////////    
#endif
    return 0;	
}
////////////////////
unsigned int neroshop::Seller::get_ratings_count() const {
    /*DB::Sqlite3 db("neroshop.db");
    if(db.table_exists("seller_ratings")) {
        unsigned int ratings_count = db.get_column_integer("seller_ratings", "COUNT(*)", "seller_id = " + std::to_string(get_id()));
        return ratings_count;
    }
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
    if(!DB::Postgres::get_singleton()->table_exists("seller_ratings")) { return 0;}
    unsigned int ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM seller_ratings WHERE seller_id = $1", { std::to_string(get_id()) });    
	
    return ratings_count;
    ////////////////////////////////
#endif    
    return 0;
}
////////////////////
unsigned int neroshop::Seller::get_total_ratings() const {
    return get_ratings_count();
}
////////////////////
unsigned int neroshop::Seller::get_reputation() const {
    /*DB::Sqlite3 db("neroshop.db");
    if(db.table_exists("seller_ratings")) {
        unsigned int ratings_count = db.get_column_integer("seller_ratings", "COUNT(*)", "seller_id = " + std::to_string(get_id()));
        if(ratings_count == 0) return 0; // seller has not yet been rated so his/her reputation will be 0%
        // get seller's good ratings
        unsigned int good_ratings = db.get_column_integer("seller_ratings", "COUNT(score)", "seller_id = " + std::to_string(get_id()) + " AND score = " + std::to_string(1));
        // calculate seller reputation
        double reputation = (good_ratings / static_cast<double>(ratings_count)) * 100;
        return static_cast<int>(reputation); // convert reputation to an integer
    }
    db.close();
    return 0;*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
	if(!DB::Postgres::get_singleton()->table_exists("seller_ratings")) { return 0;}
    unsigned int ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM seller_ratings WHERE seller_id = $1", { std::to_string(get_id()) });
    if(ratings_count == 0) { return 0;} // seller has not yet been rated so his or her reputation will be 0%
    // get seller's good ratings
    unsigned int good_ratings = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { std::to_string(get_id()), std::to_string(1) });
    
    // calculate seller reputation
    double reputation = (good_ratings / static_cast<double>(ratings_count)) * 100;
    return static_cast<int>(reputation); // convert reputation to an integer (for easier readability)
	////////////////////////////////
#endif
    return 0;	
}
////////////////////
std::vector<unsigned int> neroshop::Seller::get_top_rated_sellers(unsigned int limit) {
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // get n seller_ids with the most positive (good) ratings
    // ISSUE: both seller_4 and seller_1 have the same number of 1_score_values but seller_1 has the highest reputation and it places seller_4 first [solved - by using reputation in addition]
    std::string command = "SELECT users.id FROM users JOIN seller_ratings ON users.id = seller_ratings.seller_id WHERE score = 1 GROUP BY users.id ORDER BY COUNT(score) DESC LIMIT $1;";
    std::vector<const char *> param_values = { std::to_string(limit).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
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
        unsigned int ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM seller_ratings WHERE seller_id = $1", { std::to_string(seller_id) });
        if(ratings_count == 0) continue; // seller has not yet been rated so his or her reputation will be 0%. Skip this seller
        int good_ratings = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(score) FROM seller_ratings WHERE seller_id = $1 AND score = $2", { std::to_string(seller_id), std::to_string(1) });
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
// getters - wallet-related stuff
////////////////////
neroshop::Wallet * neroshop::Seller::get_wallet() const {
    return wallet.get();
}
////////////////////
////////////////////
////////////////////
// getters - order-related stuff
////////////////////
unsigned int neroshop::Seller::get_customer_order(unsigned int index) const {
    if(customer_order_list.empty()) return 0;//return nullptr;
    if(index > (customer_order_list.size() - 1)) throw std::out_of_range("neroshop::Seller::get_customer_order(): attempt to access invalid index");
    return customer_order_list[index];
}
////////////////////
unsigned int neroshop::Seller::get_customer_order_count() const {
    return customer_order_list.size();
}
////////////////////
std::vector<int> neroshop::Seller::get_pending_customer_orders() {
#if defined(NEROSHOP_USE_POSTGRESQL)
    std::vector<int> pending_order_list;
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////////////////
    // update customer_order_list (by adding any new orders or orders that have not yet been added)
    update_customer_orders();
    ////////////////////////////////
    // now lets get all the pending orders from the UPDATED customer_order_list  
    std::string pending_order_msg = "Pending orders awaiting seller approval: (ids: ";
    for(int i = 0; i < customer_order_list.size(); i++) {
         int pending_orders = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM orders WHERE id = $1 AND status = $2", { std::to_string(customer_order_list[i]), "Pending" });
         if(pending_orders != 0) {
             pending_order_list.push_back(customer_order_list[i]);
             // gather ids of pending_orders
             pending_order_msg += std::to_string(pending_orders) + "; ";
         }
    }
    pending_order_msg.append(")");
    pending_order_msg = String::remove_last_of(pending_order_msg, "; ");
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
unsigned int neroshop::Seller::get_sales_count() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // should item not be considered sold until the order is done processing or nah ?
	int items_sold = DB::Postgres::get_singleton()->get_integer_params("SELECT SUM(item_qty) FROM order_item WHERE seller_id = $1;", { std::to_string(get_id()) });
	return items_sold;
#endif
    return 0;	
}
////////////////////
unsigned int neroshop::Seller::get_units_sold(unsigned int item_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    int units_sold = DB::Postgres::get_singleton()->get_integer_params("SELECT SUM(item_qty) FROM order_item WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(get_id()) });
    return units_sold;
#endif
    return 0;    
}
////////////////////
unsigned int neroshop::Seller::get_units_sold(const neroshop::Item& item) const {
    return get_units_sold(item.get_id());
}
////////////////////
double neroshop::Seller::get_sales_profit() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    double profit_from_sales = DB::Postgres::get_singleton()->get_real_params("SELECT SUM(item_price * item_qty) FROM order_item WHERE seller_id = $1;", { std::to_string(get_id()) });//neroshop::print("The overall profit made from all sales combined is: $" + std::to_string(profit_from_sales), 3);
    return profit_from_sales;
#endif    
    return 0.0;
}
////////////////////
double neroshop::Seller::get_profits_made(unsigned int item_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    double item_profits = DB::Postgres::get_singleton()->get_real_params("SELECT SUM(item_price * item_qty) FROM order_item WHERE item_id = $1 AND seller_id = $2;", { std::to_string(item_id), std::to_string(get_id()) });//std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });neroshop::print("The overall profit made from \"" + item_name + "\" is: $" + std::to_string(item_profits), 3);
    return item_profits;
#endif    
    return 0.0;
}
////////////////////
double neroshop::Seller::get_profits_made(const neroshop::Item& item) const {
    return get_profits_made(item.get_id());
}
////////////////////
unsigned int neroshop::Seller::get_item_id_with_most_sales() const { // this function is preferred over the "_by_mode" version as it provides the most accurate best-selling item_id result
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // get the item with the biggest quantity sold (returns multiple results but I've limited it to 1)
    int item_with_biggest_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT item_id FROM order_item WHERE seller_id = $1 GROUP BY item_id ORDER BY SUM(item_qty) DESC LIMIT 1;", { std::to_string(get_id()) }); // from the biggest to smallest sum of item_qty
#ifdef NEROSHOP_DEBUG
    std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_with_biggest_qty) });
    neroshop::print("\"" + item_name + "\" is your best-selling item with a sale of " + std::to_string(get_units_sold(item_with_biggest_qty)) + " units", 3);
#endif    
    return item_with_biggest_qty;
#endif
    return 0;    
}
////////////////////
unsigned int neroshop::Seller::get_item_id_with_most_orders() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // get the item with the most occurences in all orders - if two items are the most occuring then it will select the lowest item_id of the two (unless I add DESC)
    int item_with_most_occurrences = DB::Postgres::get_singleton()->get_integer_params("SELECT MODE() WITHIN GROUP (ORDER BY item_id) FROM order_item WHERE seller_id = $1;", { std::to_string(get_id()) });
#ifdef NEROSHOP_DEBUG
    std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_with_most_occurrences) });
    int times_occured = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM order_item WHERE item_id = $1 AND seller_id = $2;", { std::to_string(item_with_most_occurrences), std::to_string(get_id()) });
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
bool neroshop::Seller::has_listed(unsigned int item_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
	bool listed = (DB::Postgres::get_singleton()->get_text_params("SELECT EXISTS(SELECT item_id FROM inventory WHERE item_id = $1 AND seller_id = $2);", { std::to_string(item_id), std::to_string(get_id()) }) == "t") ? true : false;
	return listed;
#endif
    return false;	
}
////////////////////
bool neroshop::Seller::has_listed(const neroshop::Item& item) const {
    return has_listed(item.get_id());
}
////////////////////
bool neroshop::Seller::has_stock(unsigned int item_id) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    bool in_stock = (DB::Postgres::get_singleton()->get_text_params("SELECT EXISTS(SELECT item_id FROM inventory WHERE item_id = $1 AND seller_id = $2 AND stock_qty > 0);", { std::to_string(item_id), std::to_string(get_id()) }) == "t") ? true : false;
    return in_stock;
#endif
    return false;    
}
////////////////////
bool neroshop::Seller::has_stock(const neroshop::Item& item) const {
    return has_stock(item.get_id());
}
////////////////////
bool neroshop::Seller::has_wallet() const {
    if(!wallet.get()) return false; // wallet is nullptr
    if(!wallet->get_monero_wallet()) return false; // wallet not opened
    return true;
}
////////////////////
bool neroshop::Seller::has_wallet_synced() const {
    if(!wallet.get()) return false; // wallet is nullptr
    if(!wallet->get_monero_wallet()) return false; // wallet not opened
    if(!wallet->get_monero_wallet()->is_synced()) return false; // wallet not synced to daemon
    return true;
}
////////////////////
////////////////////
////////////////////
// callbacks
////////////////////
neroshop::User * neroshop::Seller::on_login(const std::string& username) { // assumes user data already exists in database
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");//DB::Postgres::get_singleton()->connect;//DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    // create a new user (seller)
    neroshop::User * user = new Seller(username);
    // set user properties retrieved from database
    dynamic_cast<Seller *>(user)->set_logged(true); // protected, so can only be accessed by child class obj   // if validator::login(user, pw) returns true, then set neroshop::User::logged to true    
#if defined(NEROSHOP_USE_POSTGRESQL)    
    int user_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM users WHERE name = $1", { username });
    dynamic_cast<Seller *>(user)->set_id(user_id);
#endif    
    dynamic_cast<Seller *>(user)->set_account_type(user_account_type::seller);
    //
    // save user to global static object for easy access
    //User::set_singleton(*user);
    //-------------------------------
    // load orders
    dynamic_cast<Seller *>(user)->load_orders();
    // load wishlists
    dynamic_cast<Seller *>(user)->load_favorites();    
    // load customer_orders
    static_cast<Seller *>(user)->load_customer_orders();
    // load cart (into memory)
    if(user->is_registered()) {
        user->get_cart()->load_cart(user->get_id());
    }        
#ifdef NEROSHOP_DEBUG
    std::cout << "\033[1;34m(account_type: " << String::lower(user->get_account_type_string()) << ", id: " << user->get_id() << ", reputation: " << static_cast<Seller *>(user)->get_reputation() << ")\033[0m" << std::endl; // get_reputation() also opens the database hence the warning
#endif    
    return user;          
}
////////////////////
void neroshop::Seller::on_order_received(std::string& subaddress) {
    if(!wallet.get()) throw std::runtime_error("wallet has not been initialized");
    if(!wallet->get_monero_wallet()) throw std::runtime_error("monero_wallet_full is not opened");
    // if wallet is not properly synced with the daemon, you can only generate used addresses
    // unless wallet is synced to a daemon, you will not be able to generate any unique addresses
    if(!wallet->get_monero_wallet()->is_synced()) throw std::runtime_error("wallet is not synced with a daemon"); // Indicates if the wallet is synced with the daemon.
    // generate 10 new subaddress after each order (just to be sure there are enough unused subaddresses to choose from)
    for(int i = 0; i < 10; i++) wallet->address_new();
    // get a list of all unused subaddresses
    std::vector<std::string> unused_subaddress_list = wallet->address_unused();
    // now pick from the list of unused subaddresses (random)
	std::random_device rd; // Generating random numbers with C++11's random requires an engine and a distribution.
    std::mt19937 mt(rd()); // This is an engine based on the Mersenne Twister 19937 (64 bits):
    std::uniform_real_distribution<double> dist(0, unused_subaddress_list.size() - 1);
    subaddress = unused_subaddress_list[static_cast<int>(dist(mt))];
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
