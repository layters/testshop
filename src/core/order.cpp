#include "order.hpp"

////////////////////
neroshop::Order::Order() : id(0) {}
////////////////////
neroshop::Order::Order(unsigned int id) {
    this->id = id; // once an order has a valid id, then it means it is already in the database
}
////////////////////
neroshop::Order::~Order() {
#ifdef NEROSHOP_DEBUG0
    std::cout << "order deleted\n";
#endif    
}
////////////////////
void neroshop::Order::create_order(const neroshop::Cart& cart, const std::string& shipping_address, std::string contact_info) {
    if(cart.get_id().empty()) { // invalid id or zero
        neroshop::print("creating a guest user order ...", 3);
        create_guest_order(cart, shipping_address, contact_info);
        return; // exit function
    }
    neroshop::print("creating a registered user order ...", 4);
    create_registered_user_order(cart, shipping_address, contact_info);
}
////////////////////
void neroshop::Order::create_guest_order(const neroshop::Cart& cart, const std::string& shipping_address, std::string contact_info) // order: order_id, [order_date], product, SKU, quantity, price (subtotal), discount (optional), shipping_cost/estimated_delivery, payment method:monero[xmr], total
{
    // check if order is not already in the database
    if(this->id > 0) { neroshop::print("This order (id: " + std::to_string(this->id) + ") already exists"); return; }
    // check if cart is empty
    ////if(cart.is_empty()) {neroshop::print("You cannot place an order: (Cart is empty)", 1);return;}// if cart is empty, exit function
    // this is a guest cart so the user id will be zero by default
    unsigned int user_id = 0;
    // seller_id cannot buy from him or her self
    // if(user_id == seller_id) { neroshop::print("You cannot buy from yourself") return; }
    // seller get all details of an order from buyer
    /*neroshop::DB::SQLite3 db("neroshop.db");
    //db.execute("PRAGMA journal_mode = WAL;"); // this may reduce the incidence of SQLITE_BUSY errors (such as database being locked) // https://www.sqlite.org/pragma.html#pragma_journal_mode
    if(!db.table_exists("orders")) { // ORDER is a keyword in sqlite :O
        db.table("orders"); // a unique id will automatically be made for each order, so long as the same db is being used
        // order-specific
        db.column("orders", "ADD", "date", "TEXT"); // date and time order was created
        db.column("orders", "ADD", "status", "TEXT"); // order status // for each individual item // you don't need all these once you have the item_id (qty, price, weight, sku_code, etc.)
        db.column("orders", "ADD", "user_id", "INTEGER"); // customer that is placing the order
        db.column("orders", "ADD", "weight", "REAL"); // total weight of all items
        db.column("orders", "ADD", "subtotal", "REAL"); // price of all items in cart (combined)
        db.column("orders", "ADD", "discount", "REAL"); // discount from coupon (optional)(price will be reduced by discount)
        //db.column("orders", "ADD", "shipping_address", "TEXT"); // will not be saved in db (for the sake of user's privacy), but will be sent to the seller // shipping_address - name, address, city, region, postal code, country
        //db.column("orders", "ADD", "shipping_method", "TEXT"); // 1 day, 2 day, 3 day, 1 week, etc.
        db.column("orders", "ADD", "shipping_cost", "REAL");
        db.column("orders", "ADD", "total", "REAL"); // total cost of all items plus shipping costs (taxes don't exist in the cryptocurrency world except in the form of tx fees)
        db.column("orders", "ADD", "payment_method", "TEXT"); //  cash, card, crypto, etc.
        db.column("orders", "ADD", "currency", "TEXT"); //  xmr, btc, usd, etc.
        db.column("orders", "ADD", "notes", "TEXT"); // must be encrypted since storing this in db
        //db.column("orders", "ADD", "", ""); // monero_address (of receipient)?
        //db.index("idx_orders_code", "orders", "code");// code can be sku, upc, ean, etc. //db.execute("CREATE UNIQUE INDEX idx_orders_sku ON Orders (sku);");
    }
    double weight = cart.get_total_weight();
    db.insert("orders", "date, status, user_id, weight, subtotal, discount, shipping_cost, total, payment_method, currency, notes",
        DB::SQLite3::to_sql_string(date)           + ", " + 
        DB::SQLite3::to_sql_string("Incomplete")      + ", " + // status
        std::to_string(user_id)           + ", " + 
        std::to_string(weight)            + ", " + 
        std::to_string(0.000000000000)    + ", " + // subtotal
        std::to_string(0.000000000000)    + ", " + // discount (overall)
        std::to_string(0.000000000000)    + ", " + // shipping_cost
        std::to_string(0.000000000000)    + ", " + // total
        DB::SQLite3::to_sql_string("crypto")       + ", " + 
        DB::SQLite3::to_sql_string("monero (xmr)") + ", " + 
        DB::SQLite3::to_sql_string("") //shipping_address//+ ", " + ", " + // notes
    );
    /////////////////////////////////////////
    if(!db.table_exists("order_item")) {
        db.table("order_item");
        db.column("order_item", "ADD", "order_id", "INTEGER");  // the order id in which the order_item belongs to
        db.column("order_item", "ADD", "item_id", "INTEGER");   // the order_item's item_id
        db.column("order_item", "ADD", "seller_id", "INTEGER"); // the seller that is selling the item_id                
        db.column("order_item", "ADD", "item_qty", "INTEGER");  // the quantity of the item the buyer wishes to buy
        db.column("order_item", "ADD", "item_price", "REAL");   // might not need the item price at all //db.column("order_item", "ADD", "", "");
    }
    // get the most recent order_id (last record from table orders)
    unsigned int order_id = db.get_column_integer("orders ORDER BY id DESC LIMIT 1", "*");//db.execute("SELECT * FROM orders ORDER BY id DESC LIMIT 1;");
    set_id(order_id); // save the order_id
    double subtotal = 0.00, discount = 0.00, shipping_cost = 0.00;
    std::string seller_currency;
    //if(!neroshop::Converter::is_supported_currency(currency)) currency = "usd"; // default
    for(int i = 0; i < cart.get_contents_count(); i++) {
        neroshop::Item * item = cart.get_item(i);
        std::string item_name = cart.get_item(i)->get_name();
        unsigned int item_id  = cart.get_item(i)->get_id();
        unsigned int item_qty = cart.get_item(i)->get_quantity();
        // if seller_id is not specified (0), then choose a random seller who is selling the same product
        int seller_id = db.get_column_integer("inventory", "seller_id", "item_id = " + std::to_string(item_id));
        if(seller_id <= 0) { std::cout << "item seller not found"; return; }     
        // if the buyer is also the seller XD
        if(user_id == seller_id) {neroshop::print("You cannot buy from yourself", 1); return;}
        // get the currency that item is priced in
        seller_currency = db.get_column_text("inventory", "currency", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));
        // get seller_price
        // if seller does not specify a price for their item, set the item price to its original price
        double seller_price = db.get_column_real("inventory", "seller_price", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));//NEROSHOP_TAG_OUT std::cout << "Seller's price for item (id: " << item_id << ") per unit is: " << seller_price << " " << seller_currency << std::endl;
        double item_price = (seller_price > 0.00) ? seller_price : item->get_price(); // set the item price to the seller's price
        // calculate subtotal (price of all items combined)
        subtotal += item_qty * item_price;
        // get seller discount
        // if seller does not specify a discount for their item, set the item discount to its original discount which is 0
        double seller_discount = db.get_column_real("inventory", "seller_discount", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));
        unsigned int discounted_items = db.get_column_integer("inventory", "discount_qty", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));
        if(seller_discount > 0.00) {
            discount += (item_qty / discounted_items) * seller_discount;
            NEROSHOP_TAG_OUT std::cout << "\033[1;37m" << "for every " << discounted_items << " " << item_name << "s, you get " << neroshop::Converter::get_currency_symbol(seller_currency) << std::fixed << std::setprecision(2) << seller_discount << " off (since you have x" << item_qty << ", total discount is: " << neroshop::Converter::get_currency_symbol(seller_currency) << ((item_qty / discounted_items) * seller_discount) << ")\033[0m" << std::endl;
        }
        // get condition of item based on seller
        std::string item_condition = db.get_column_text("inventory", "condition", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));
        // check again if item is still in stock
        if(!item->in_stock()) {
            neroshop::print("[error]: order failed  [reason]: The following item is out of stock: " + item_name);
            set_status(order_status::failed); // set order status to failed
            db.update("orders", "status", DB::SQLite3::to_sql_string(get_status_string()), "id = " + std::to_string(order_id));
            return; // exit function //continue; // skip this item
        }
        // add each item to the same order_id
        db.insert("order_item", "order_id, item_id, seller_id, item_qty, item_price",
            std::to_string(order_id) + ", " + std::to_string(item_id) + ", " + std::to_string(seller_id) + ", " + std::to_string(item_qty) + ", " + std::to_string(item_price));
        // reduce stock_qty of each purchased item (subtract stock_qty by item_qty that buyer is purchasing)
        int stock_qty = db.get_column_integer("inventory", "stock_qty", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));//std::cout << "stock_qty of item BEFORE deletion: " << stock_qty << std::endl; 
        db.update("inventory", "stock_qty", std::to_string(stock_qty - item_qty), "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));//std::cout << "stock_qty of item AFTER deletion: " << db.get_column_integer("inventory", "stock_qty", "item_id = " + std::to_string(item->get_id()) + " AND seller_id = " + std::to_string(seller_id)) << std::endl;
        // If stock qty goes to 0, delete this item from inventory (row) - bad idea because once an item is listed by a seller then the listing is permanent unless the seller's account is deleted or the manufacture of the item has discontinued
        //stock_qty = db.get_column_integer("inventory", "stock_qty", "item_id = " + std::to_string(item->get_id()) + " AND seller_id = " + std::to_string(seller_id));
        //if(stock_qty <= 0) db.drop("inventory", "stock_qty = " + std::to_string(0));
    }
    // print order message
    neroshop::print("Thank you for using neroshop.");
    neroshop::io_write("You have ordered: ");
    for(int i = 0; i < cart.get_contents_count(); i++) {
        neroshop::Item * item = cart.get_item(i);
        std::cout << "\033[0;94m" + item->get_name() << " (x" << item->get_quantity() << ")\033[0m" << std::endl;
        item->set_quantity(0); // reset all item quantity to 0 (now that order has been completed)
    }
    // empty cart after completing order
    cart.empty();
    // if a user loses internet connection, libcurl cannot get the exchange rate so the order will fail
    // todo: find a way to check if user has internet connection
    set_status(order_status::failed); // set order status to failed by default
    db.update("orders", "status", DB::SQLite3::to_sql_string(get_status_string()), "id = " + std::to_string(order_id));
    // update order details - converts seller's currency of choice to xmr, the moment you create an order
    db.update("orders", "subtotal", std::to_string(neroshop::Converter::convert_xmr(subtotal, seller_currency,  true)), "id = " + std::to_string(order_id));
    db.update("orders", "discount", std::to_string(neroshop::Converter::convert_xmr(discount, seller_currency,  true)), "id = " + std::to_string(order_id));
    db.update("orders", "shipping_cost", std::to_string(neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true)), "id = " + std::to_string(order_id));
    double total = (subtotal - discount) + shipping_cost;
    db.update("orders", "total", std::to_string(neroshop::Converter::convert_xmr(total, seller_currency,  true)), "id = " + std::to_string(order_id));
    // display order details
    std::string currency = Script::get_string(neroshop::get_lua_state(), "neroshop.currency");
    if(currency.empty() || !neroshop::Converter::is_supported_currency(currency)) currency = "usd"; // default //neroshop::Converter::from_xmr(neroshop::Converter::to_xmr(, seller_currency), currency);
    neroshop::print("Sit tight as we notify the seller(s) about your order.");
    std::cout << "Subtotal: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(subtotal, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(subtotal, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl; // (() ? : "")
    if(discount > 0) std::cout << "Discount: -" << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(discount, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(discount, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    std::cout << "Shipping: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    std::cout << "Order total: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(total, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(total, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    //std::cout << "Estimated delivery date: " << delivery_date_est << std::endl;
    set_status(order_status::pending); // if everything went well then order will be set to pending
    db.update("orders", "status", DB::SQLite3::to_sql_string(get_status_string()), "id = " + std::to_string(order_id));
    /////////////////////////////////////////
    db.close();*/ // always close db when done
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    float weight = cart.get_total_weight();
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    // begin transaction
    DB::Postgres::get_singleton()->execute("BEGIN;");
    if(!DB::Postgres::get_singleton()->table_exists("orders")) { 
        DB::Postgres::get_singleton()->create_table("orders");
        // order-specific
        DB::Postgres::get_singleton()->add_column("orders", "user_id", "integer REFERENCES users(id)"); // customer that is placing the order
        DB::Postgres::get_singleton()->add_column("orders", "creation_date", "timestamptz DEFAULT CURRENT_TIMESTAMP"); // date and time order was created
        DB::Postgres::get_singleton()->add_column("orders", "status", "text"); // order status // for each individual item // you don't need all these once you have the item_id (qty, price, weight, sku_code, etc.)        
        DB::Postgres::get_singleton()->add_column("orders", "weight", "real"); // total weight of all items
        DB::Postgres::get_singleton()->add_column("orders", "subtotal", "numeric(20, 12)"); // 20 - 12 = 8 total digits (on left side / whole number side) // 12 decimals // monero supply is 18 million so that is an 8 digit-number // price of all items in cart (combined)
        DB::Postgres::get_singleton()->add_column("orders", "discount", "numeric(20, 12)"); // discount from coupon (optional)(price will be reduced by discount)
        //DB::Postgres::get_singleton()->column("orders", "ADD", "shipping_address", "TEXT"); // will not be saved in db (for the sake of user's privacy), but will be sent to the seller // shipping_address - name, address, city, region, postal code, country
        //DB::Postgres::get_singleton()->column("orders", "ADD", "shipping_method", "TEXT"); // 1 day, 2 day, 3 day, 1 week, etc.
        DB::Postgres::get_singleton()->add_column("orders", "shipping_cost", "numeric(20, 12)");
        DB::Postgres::get_singleton()->add_column("orders", "total", "numeric(20, 12)"); // total cost of all items plus shipping costs (taxes don't exist in the cryptocurrency world except in the form of tx fees)
        DB::Postgres::get_singleton()->add_column("orders", "payment_method", "text"); //  cash, card, crypto, etc.
        DB::Postgres::get_singleton()->add_column("orders", "currency", "text"); //  xmr, btc, usd, etc.
        DB::Postgres::get_singleton()->add_column("orders", "notes", "text"); // must be encrypted since storing this in db
        //db.column("orders", "ADD", "", ""); // monero_address (of receipient)?
        //db.index("idx_orders_code", "orders", "code");// code can be sku, upc, ean, etc. //db.execute("CREATE UNIQUE INDEX idx_orders_sku ON Orders (sku);");
    }
    // before we create the order, lets create a savepoint (just in case the order fails or we screw something up)
    DB::Postgres::get_singleton()->execute("SAVEPOINT before_order_creation_savepoint;");
    // set order date (timestamp) to the current date (timestamp) // timestamp includes both date and time instead of the time only
    std::string date = DB::Postgres::get_singleton()->get_text("SELECT CURRENT_TIMESTAMP;"); // SELECT NOW(), CURRENT_TIMESTAMP; // both now() and current_timestamp are the same thing and both include the timezone: https://dba.stackexchange.com/questions/63548/difference-between-now-and-current-timestamp    // NOW() is postgresql's version; CURRENT_TIMESTAMP is an SQL-standard
    // insert new order
    int order_id = DB::Postgres::get_singleton()->get_integer_params("INSERT INTO orders (user_id, creation_date, status, weight, subtotal, discount, shipping_cost, total, payment_method, currency, notes) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11) RETURNING id"/*, 12, 13, 14, 15)"*/, 
        { std::to_string(user_id), date, "Incomplete", std::to_string(weight), std::to_string(0.000000000000), std::to_string(0.000000000000), std::to_string(0.000000000000), std::to_string(0.000000000000),
        "crypto", "monero (xmr)", shipping_address + ";" + contact_info }); // shipping_address, shipping_nethod ??
    std::cout << "order_id: " << order_id << std::endl; // temp
    /////////////////////////////////////////
    if(!DB::Postgres::get_singleton()->table_exists("order_item")) {
        DB::Postgres::get_singleton()->create_table("order_item");
        DB::Postgres::get_singleton()->add_column("order_item", "order_id", "integer REFERENCES orders(id) ON DELETE CASCADE");  // the order id in which the order_item belongs to
        DB::Postgres::get_singleton()->add_column("order_item", "item_id", "integer REFERENCES item(id)");     // the order_item's item_id
        DB::Postgres::get_singleton()->add_column("order_item", "seller_id", "integer REFERENCES users(id)");  // the seller that is selling the item_id                
        DB::Postgres::get_singleton()->add_column("order_item", "item_qty", "integer");  // the quantity of the item the buyer wishes to buy
        DB::Postgres::get_singleton()->add_column("order_item", "item_price", "numeric(20, 12)");   // might not need the item price at all //db.column("order_item", "ADD", "", "");
    }
    // get the most recent or last recorded order_id made by this specific user
    // BUT what if the most recent order was done by another user at the same time we created our order ??
    // we would have to use "WHERE user_id = $1" to be more specific about which latest order we want
    ////int order_id = DB::Postgres::get_singleton()->get_integer_params("SELECT * FROM orders WHERE user_id = $1 ORDER BY id DESC LIMIT 1", { std::to_string(user_id) }); // DESC means from the bottom-to-top (biggest-to-smallest) while ASC means from top-to-bottom // LIMIT 1 means we want only 1 result from the select statement    //std::cout << "order_id (latest WHERE user_id = this): " << order_id << std::endl;
    /////////////////
    // testing here (temporary)
    //std::cout << "\033[36morder_date: " << date << "\033[0m\n";
    std::cout << "current_timestamp: " << DB::Postgres::get_singleton()->get_text("SELECT NOW();") << std::endl;
    // convert local_time to utc
    std::cout << "\033[36muniversal_timestamp (UTC): " << DB::Postgres::get_singleton()->get_text_params("SELECT $1::timestamptz AT TIME ZONE 'UTC'", { date }) << "\033[0m\n";; // https://www.worldtimebuddy.com/est-to-utc-converter // "SELECT TIMESTAMP WITH TIME ZONE '2022-02-11 7:00:00-05' AT TIME ZONE 'UTC';" is the same as "SELECT TIMESTAMPTZ '2022-02-11 7:00:00-05' AT TIME ZONE 'UTC';"// 20:38 (8:00PM EST) would be 1:38 AM UTC time
    // check if a date has passed (or expired)
    // Ball(id:1) from seller layter(id:4)
    // EXPIRED = 0 OR > 0
    // VALID = NEGATIVE
    /*std::string discount_exp = DB::Postgres::get_singleton()->get_text_params("SELECT discount_expiry FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(1), std::to_string(4) });
    std::cout << "Ball discount exp_date: \033[0;35m" << discount_exp << "\033[0m" << std::endl;
    std::cout << "Ball discount exp_date (UTC): \033[0;35m" << DB::Postgres::get_singleton()->localtimestamp_to_utc(discount_exp) << "\033[0m" << std::endl;*/
    /////////////////
    this->id = order_id; // save the order_id
    double subtotal = 0.00, discount = 0.00, shipping_cost = 0.00;
    std::string seller_currency;
    //if(!neroshop::Converter::is_supported_currency(currency)) currency = "usd"; // default
    for(int i = 0; i < cart.get_contents_count(); i++) {
        neroshop::Item * item = cart.get_item(i);
        unsigned int item_id  = cart.get_item(i)->get_id();
        std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });//cart.get_item(i)->get_name();
        unsigned int item_qty = cart.get_item(i)->get_quantity(0);//DB::Postgres::get_singleton()->get_integer("SELECT item_qty FROM cart or cart_item WHERE item_id = $1", { std::to_string(item_id) });
        // if seller_id is not specified (0), then choose a random seller who is selling the same product, but it MUST be in stock!!
        int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) });
        if(seller_id == 0) { std::cout << "item seller not found"; DB::Postgres::get_singleton()->execute("ROLLBACK TO before_order_creation_savepoint;"); /*DB::Postgres::get_singleton()->finish();*/ return; }
        // if the buyer is also the seller XD
        if(user_id == seller_id) {neroshop::print("You cannot buy from yourself", 1); DB::Postgres::get_singleton()->execute("ROLLBACK TO before_order_creation_savepoint;"); /*DB::Postgres::get_singleton()->finish();*/ return;} // go back to savepoint, before the order was even create to prevent us from inserting a new order
        // get the currency that item is priced in
        seller_currency = DB::Postgres::get_singleton()->get_text_params("SELECT currency FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        // get seller_price
        // if seller does not specify a price for their item, set the item price to its original price
        double seller_price = DB::Postgres::get_singleton()->get_double_params("SELECT seller_price FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });//NEROSHOP_TAG_OUT std::cout << "Seller's price for item (id: " << item_id << ") per unit is: " << seller_price << " " << seller_currency << std::endl;
        double item_price = (seller_price > 0.00) ? seller_price : item->get_price(); // set the item price to the seller's price
        // calculate subtotal (price of all items combined)
        subtotal += item_qty * item_price;
        // get seller discount
        // if seller does not specify a discount for their item, set the item discount to its original discount which is 0
        double seller_discount = DB::Postgres::get_singleton()->get_double_params("SELECT seller_discount FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        unsigned int discounted_items = DB::Postgres::get_singleton()->get_integer_params("SELECT discount_qty FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        unsigned int discount_times = DB::Postgres::get_singleton()->get_integer_params("SELECT discount_times FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        if(seller_discount > 0.00) { // && discount_times > 0
            // check if discount has expired
            // if seconds >= 0 then you are at the current time of expiration (meaning it expires NOW)
            // negative seconds means it has NOT yet expired
            // get days that have passed since EXP date
            // THIS CODE IS LIKELY WRONG (WELL, THE TIME/DATE CALCULATION PART)               
            // fetch discount expiration date
            std::string discount_exp_date = DB::Postgres::get_singleton()->get_text_params("SELECT discount_expiry FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) }); 
            // convert discount exp_date from localtime to utc // then get the time difference between now and the expiration date
            int timestamp_since_exp_date = std::stoi(DB::Postgres::get_singleton()->get_text_params("SELECT extract (second from (timezone('utc', now()) - $1))", { DB::Postgres::get_singleton()->localtimestamp_to_utc(discount_exp_date) }));//stoi(DB::Postgres::get_singleton()->get_text_params("SELECT NOW() - $1::timestamptz", { discount_exp_date }));//DB::Postgres::get_singleton()->get_text_params("SELECT DATE_TRUNC('second', timezone('utc', now()) - $1)", { DB::Postgres::get_singleton()->localtimestamp_to_utc(discount_exp) })//prints HH:MM:SS
            std::cout << "\033[0;35mtime since expiration date: " << timestamp_since_exp_date << "s\033[0m" << std::endl;
            if(timestamp_since_exp_date <  0) {
                neroshop::print("Discount is valid until " + discount_exp_date, 3);        
                // *** here, the discount only applies to "discounted_items * number_of_times_discount_can_be_used" ***        
                //std::cout << "This discount may only be used " << discounted_times << " times\n";
                // (number of items to apply discount to) x (number of times you can use this discount per order or until discount expires)
                int item_qty_reduced = discounted_items * discount_times;
                discount += (item_qty_reduced / discounted_items) * seller_discount;
                NEROSHOP_TAG_OUT std::cout << "\033[1;37m" << "for every " << discounted_items << " " << item_name << "s, you get " << neroshop::Converter::get_currency_symbol(seller_currency) << std::fixed << std::setprecision(2) << seller_discount << " off (out of your x" << item_qty << ", only \033[0;93mx" << item_qty_reduced << "\033[1;37m will receive the total discount: \033[0;93m" << neroshop::Converter::get_currency_symbol(seller_currency) << ((item_qty_reduced / discounted_items) * seller_discount) << " \033[1;37m(number of times discount applied: " << discount_times << " time(s) on this particular item))\033[0m" << std::endl;            
                // *** here, the discount applies to all of this item's qty ***
                //discount += (item_qty / discounted_items) * seller_discount;
                //NEROSHOP_TAG_OUT std::cout << "\033[1;37m" << "for every " << discounted_items << " " << item_name << "s, you get " << neroshop::Converter::get_currency_symbol(seller_currency) << std::fixed << std::setprecision(2) << seller_discount << " off (since you have x" << item_qty << ", total discount is: " << neroshop::Converter::get_currency_symbol(seller_currency) << ((item_qty / discounted_items) * seller_discount) << ")\033[0m" << std::endl;            
            }
            if(timestamp_since_exp_date >= 0) neroshop::print("Discount has expired", 1);
        }
        // get condition of item based on seller
        std::string item_condition = DB::Postgres::get_singleton()->get_text_params("SELECT condition FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        // check again if item is still in stock
        bool item_in_stock = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) });
        if(!item_in_stock) {
            neroshop::print("[error]: order failed  [reason]: The following item is out of stock: " + item_name);
            // set order status to failed
            set_status(order_status::failed);
            DB::Postgres::get_singleton()->execute_params("UPDATE orders SET status = $1 WHERE id = $2", { get_status_string(), std::to_string(order_id) });
            // revert everything to the point before we created the order
            neroshop::print("reverting order creation ...", 3);
            DB::Postgres::get_singleton()->execute("ROLLBACK TO before_order_creation_savepoint;");
            /*DB::Postgres::get_singleton()->finish();*/
            return; // exit function //continue; // skip this item
        }
        // add each item to the same order_id
        DB::Postgres::get_singleton()->execute_params("INSERT INTO order_item (order_id, item_id, seller_id, item_qty, item_price) "
            "VALUES ($1, $2, $3, $4, $5)", { std::to_string(order_id), std::to_string(item_id), std::to_string(seller_id), std::to_string(item_qty), std::to_string(item_price) });
        // reduce stock_qty of each purchased item (subtract stock_qty by item_qty that buyer is purchasing)
        int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });//std::cout << "stock_qty of item BEFORE deletion: " << stock_qty << std::endl; 
        DB::Postgres::get_singleton()->execute_params("UPDATE inventory SET stock_qty = $1 WHERE item_id = $2 AND seller_id = $3", { std::to_string(stock_qty - item_qty), std::to_string(item_id), std::to_string(seller_id) });//std::cout << "stock_qty of item AFTER deletion: " << db.get_column_integer("inventory", "stock_qty", "item_id = " + std::to_string(item->get_id()) + " AND seller_id = " + std::to_string(seller_id)) << std::endl;
        // If stock qty goes to 0, delete this item from inventory (row) - bad idea because once an item is listed by a seller then the listing is permanent unless the seller's account is deleted or the manufacture of the item has discontinued
        //stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        //if(stock_qty <= 0) DB::Postgres::get_singleton()->execute_params("DELETE FROM inventory WHERE stock_qty = $1", { std::to_string(0) });
    }
    // print order message
    neroshop::print("Thank you for using neroshop.");
    neroshop::io_write("You have ordered: ");
    for(int i = 0; i < cart.get_contents_count(); i++) {
        neroshop::Item * item = cart.get_item(i);
        std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item->get_id()) });//unsigned int item_qty = db.get_column_integer("Cart", "item_qty", "item_id = " + std::to_string(item_id));
        std::cout << ((i > 0) ? "                              " : "") << "\033[0;94m" + item_name << " (x" << item->get_quantity(0) << ")\033[0m" << std::endl;
        item->set_quantity(0, 0); // reset all item quantity to 0 (now that order has been completed) // db.update("Cart", "item_qty", std::to_string(quantity), "item_id = " + std::to_string(item_id));
    }
    // empty cart after completing order
    const_cast<neroshop::Cart&>(cart).empty();
    // if a user loses internet connection, libcurl cannot get the exchange rate so the order will fail
    // todo: find a way to check if user has internet connection
    set_status(order_status::failed); // set order status to failed by default
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET status = $1 WHERE id = $2", { get_status_string(), std::to_string(order_id) });
    // update order details - converts seller's currency of choice to xmr, the moment you create an order
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET subtotal = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(subtotal, seller_currency,  true)), std::to_string(order_id) });
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET discount = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(discount, seller_currency,  true)), std::to_string(order_id) });
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET shipping_cost = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true)), std::to_string(order_id) });
    double total = (subtotal - discount) + shipping_cost;
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET total = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(total, seller_currency,  true)), std::to_string(order_id) });
    // display order details
    std::string currency = Script::get_string(neroshop::get_lua_state(), "neroshop.currency");
    if(currency.empty() || !neroshop::Converter::is_supported_currency(currency)) currency = "usd"; // default //neroshop::Converter::from_xmr(neroshop::Converter::to_xmr(, seller_currency), currency);
    neroshop::print("Sit tight as we notify the seller(s) about your order.");
    std::cout << "Subtotal: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(subtotal, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(subtotal, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl; // (() ? : "")
    if(discount > 0) std::cout << "Discount: -" << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(discount, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (-" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(discount, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    std::cout << "Shipping: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    std::cout << "Order total: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(total, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(total, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    //std::cout << "Estimated delivery date: " << delivery_date_est << std::endl;
    // set order status => pending
    set_status(order_status::pending); // if everything went well then order will be set to pending
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET status = $1 WHERE id = $2", { get_status_string(), std::to_string(order_id) });
    // end transaction
    DB::Postgres::get_singleton()->execute("COMMIT;");
    /*DB::Postgres::get_singleton()->finish();*/
    ////////////////////////////////    
    // notify seller // send seller notification for every order made that contains the item they have on sale: "You have received an order on neroshop for 1 items(s) totaling: 0.2 XMR ($107.50 at current rates)". Also include buyer's name and address and contact
    // seller receives the order made by the buyer - shipping address will be encrypted then sent to the seller
    // seller generates a unique subaddress when they accept the order, the stock_qty of item_id in table Inventory (seller can choose to either accept or deny the order, but they must give a reason for denying an order)
#endif    
}
////////////////////
////////////////////
////////////////////
////////////////////
void neroshop::Order::create_registered_user_order(const neroshop::Cart& cart, const std::string& shipping_address, std::string contact_info) {
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // check if order is already in the database
    if(id != 0) {
        neroshop::print("This order (id: " + std::to_string(id) + ") already exists"); 
        return; 
    }
    // check if id is valid first
    if(cart.get_id() == 0) {
        neroshop::print("No cart found on account", 1);
        // use local cart instead
        //create_guest_order(user_id, shipping_address);
        return; // exit current function
    }
    std::cout << "cart_id: " << cart.get_id() << std::endl;
    // get user id that this cart belongs to
    int user_id = DB::Postgres::get_singleton()->get_integer_params("SELECT user_id FROM cart WHERE id = $1;", { std::to_string(cart.get_id()) });
    std::cout << "cart belongs to user_id: " << user_id << std::endl;
    // then check if cart is empty or not
    if(cart.is_empty(user_id)) {
        neroshop::print("You cannot place an order: (Cart is empty)", 1);
        return; // if cart is empty, exit function
    }
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    ////////////////////////////////
    if(!DB::Postgres::get_singleton()->table_exists("orders")) { 
        DB::Postgres::get_singleton()->create_table("orders");
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS user_id integer REFERENCES users(id);"); // customer that is placing the order
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS creation_date timestamptz DEFAULT CURRENT_TIMESTAMP;"); // date and time order was created
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS status text;");
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS weight real;"); // total weight of all items combined
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS subtotal numeric(20, 12);"); // price of all items in cart (combined)
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS discount numeric(20, 12);");////DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS shipping_address text;");////DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS shipping_method text;");
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS shipping_cost numeric(20, 12);");
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS total numeric(20, 12);"); // total cost of all items plus shipping costs (sales taxes don't exist in the cryptocurrency world except in the form of tx fees)
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS payment_method text;"); // crypto
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS currency text;"); // xmr
        DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS notes text;"); // encrypted message containing customer's shipping address
        //DB::Postgres::get_singleton()->execute("ALTER TABLE orders ADD COLUMN IF NOT EXISTS column_name column_type;");
    }
    // begin transaction
    DB::Postgres::get_singleton()->execute("BEGIN;");
    // before we create the order, lets create a savepoint (just in case the order fails or we screw something up)
    ////DB::Postgres::get_singleton()->execute("SAVEPOINT order_creation_savepoint;");
    // set order date (timestamp) to the current date (timestamp) // timestamp includes both date and time instead of the time only
    std::string date = DB::Postgres::get_singleton()->get_text("SELECT CURRENT_TIMESTAMP;"); // SELECT NOW(), CURRENT_TIMESTAMP; // both now() and current_timestamp are the same thing and both include the timezone: https://dba.stackexchange.com/questions/63548/difference-between-now-and-current-timestamp    // NOW() is postgresql's version; CURRENT_TIMESTAMP is an SQL-standard
    double weight = cart.get_total_weight(user_id);
    std::cout << "order_weight: " << weight << std::endl;
    // insert new order
    int order_id = DB::Postgres::get_singleton()->get_integer_params("INSERT INTO orders (user_id, creation_date, status, weight, subtotal, discount, shipping_cost, total, payment_method, currency, notes) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11) RETURNING id"/*, 12, 13, 14, 15)"*/, 
        { std::to_string(user_id), date, "Incomplete", std::to_string(weight), std::to_string(0.000000000000), std::to_string(0.000000000000), std::to_string(0.000000000000), std::to_string(0.000000000000),
        "crypto", "monero (xmr)", shipping_address + ";" + contact_info }); // shipping_address, shipping_nethod ??
    ////////////////////////////////
    std::cout << "order_id: " << order_id << std::endl; // temp
    if(order_id == 0) {
        neroshop::print("Order creation failed", 1);
        ////DB::Postgres::get_singleton()->execute("ROLLBACK TO order_creation_savepoint;");
        return;
    }   
    this->id = order_id; // save (store) the order_id 
    ////////////////////////////////
    if(!DB::Postgres::get_singleton()->table_exists("order_item")) {
        DB::Postgres::get_singleton()->create_table("order_item");
        DB::Postgres::get_singleton()->execute("ALTER TABLE order_item ADD COLUMN IF NOT EXISTS order_id integer REFERENCES orders(id) ON DELETE CASCADE;"); // the order_id in which the order_item belongs to
        DB::Postgres::get_singleton()->execute("ALTER TABLE order_item ADD COLUMN IF NOT EXISTS item_id integer REFERENCES item(id);");
        DB::Postgres::get_singleton()->execute("ALTER TABLE order_item ADD COLUMN IF NOT EXISTS seller_id integer REFERENCES users(id);"); // the seller that is selling the item_id
        DB::Postgres::get_singleton()->execute("ALTER TABLE order_item ADD COLUMN IF NOT EXISTS item_qty integer;");
        DB::Postgres::get_singleton()->execute("ALTER TABLE order_item ADD COLUMN IF NOT EXISTS item_price numeric(20, 12);"); // the quantity of the item the buyer wishes to buy
    }
    ////////////////////////////////
    ///*
    double subtotal = 0.00, discount = 0.00, shipping_cost = 0.00;
    std::string seller_currency;
    //if(!neroshop::Converter::is_supported_currency(currency)) currency = "usd"; // default
    for(int i = 0; i < cart.get_contents_count(); i++) {
        neroshop::Item * item = cart.get_item(i);
        unsigned int item_id  = item->get_id();
        std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
        unsigned int item_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT item_qty FROM cart_item WHERE cart_id = $1 AND item_id = $2", { std::to_string(cart.get_id()), std::to_string(item_id) });
        // if seller_id is not specified (0), then choose a random seller who is selling the same product, but it MUST be in stock!!
        int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) });
        if(seller_id == 0) { std::cout << "item_id is: " << item_id << "\n";std::cout << "item seller not found" << std::endl; DB::Postgres::get_singleton()->execute("ROLLBACK;"); return; }//DB::Postgres::get_singleton()->finish(); return; }
        // if the buyer is also the seller XD
        if(user_id == seller_id) {
            neroshop::print("You cannot buy from yourself", 1); 
            DB::Postgres::get_singleton()->execute("ROLLBACK;");
            return;
        }//DB::Postgres::get_singleton()->finish(); return;} // go back to savepoint, before the order was even create to prevent us from inserting a new order
        // get the currency that item is priced in
        seller_currency = DB::Postgres::get_singleton()->get_text_params("SELECT currency FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        // get seller_price
        // if seller does not specify a price for their item, set the item price to its original price
        double seller_price = DB::Postgres::get_singleton()->get_double_params("SELECT seller_price FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });//NEROSHOP_TAG_OUT std::cout << "Seller's price for item (id: " << item_id << ") per unit is: " << seller_price << " " << seller_currency << std::endl;
        double item_price = (seller_price > 0.00) ? seller_price : item->get_price(); // set the item price to the seller's price
        // calculate subtotal (price of all items combined)
        subtotal += item_qty * item_price;
        // get seller discount
        // if seller does not specify a discount for their item, set the item discount to its original discount which is 0
        double seller_discount = DB::Postgres::get_singleton()->get_double_params("SELECT seller_discount FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        unsigned int discounted_items = DB::Postgres::get_singleton()->get_integer_params("SELECT discount_qty FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        unsigned int discount_times = DB::Postgres::get_singleton()->get_integer_params("SELECT discount_times FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        if(seller_discount > 0.00) { // && discount_times > 0
            // check if discount has expired
            // if seconds >= 0 then you are at the current time of expiration (meaning it expires NOW)
            // negative seconds means it has NOT yet expired
            // get days that have passed since EXP date
            // THIS CODE IS LIKELY WRONG (WELL, THE TIME/DATE CALCULATION PART)               
            // fetch discount expiration date
            std::string discount_exp_date = DB::Postgres::get_singleton()->get_text_params("SELECT discount_expiry FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) }); 
            // convert discount exp_date from localtime to utc // then get the time difference between now and the expiration date
            int timestamp_since_exp_date = std::stoi(DB::Postgres::get_singleton()->get_text_params("SELECT extract (second from (timezone('utc', now()) - $1))", { DB::Postgres::get_singleton()->localtimestamp_to_utc(discount_exp_date) }));//stoi(DB::Postgres::get_singleton()->get_text_params("SELECT NOW() - $1::timestamptz", { discount_exp_date }));//DB::Postgres::get_singleton()->get_text_params("SELECT DATE_TRUNC('second', timezone('utc', now()) - $1)", { DB::Postgres::get_singleton()->localtimestamp_to_utc(discount_exp) })//prints HH:MM:SS
            std::cout << "\033[0;35mtime since expiration date: " << timestamp_since_exp_date << "s\033[0m" << std::endl;
            if(timestamp_since_exp_date <  0) {
                neroshop::print("Discount is valid until " + discount_exp_date, 3);        
                // *** here, the discount only applies to "discounted_items * number_of_times_discount_can_be_used" ***        
                //std::cout << "This discount may only be used " << discounted_times << " times\n";
                // (number of items to apply discount to) x (number of times you can use this discount per order or until discount expires)
                int item_qty_reduced = discounted_items * discount_times;
                discount += (item_qty_reduced / discounted_items) * seller_discount;
                NEROSHOP_TAG_OUT std::cout << "\033[1;37m" << "for every " << discounted_items << " " << item_name << "s, you get " << neroshop::Converter::get_currency_symbol(seller_currency) << std::fixed << std::setprecision(2) << seller_discount << " off (out of your x" << item_qty << ", only \033[0;93mx" << item_qty_reduced << "\033[1;37m will receive the total discount: \033[0;93m" << neroshop::Converter::get_currency_symbol(seller_currency) << ((item_qty_reduced / discounted_items) * seller_discount) << " \033[1;37m(number of times discount applied: " << discount_times << " time(s) on this particular item))\033[0m" << std::endl;            
                // *** here, the discount applies to all of this item's qty ***
                //discount += (item_qty / discounted_items) * seller_discount;
                //NEROSHOP_TAG_OUT std::cout << "\033[1;37m" << "for every " << discounted_items << " " << item_name << "s, you get " << neroshop::Converter::get_currency_symbol(seller_currency) << std::fixed << std::setprecision(2) << seller_discount << " off (since you have x" << item_qty << ", total discount is: " << neroshop::Converter::get_currency_symbol(seller_currency) << ((item_qty / discounted_items) * seller_discount) << ")\033[0m" << std::endl;            
            }
            if(timestamp_since_exp_date >= 0) neroshop::print("Discount has expired", 1);
        }
        // get condition of item based on seller
        std::string item_condition = DB::Postgres::get_singleton()->get_text_params("SELECT condition FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        // check again if item is still in stock
        bool item_in_stock = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) });
        if(!item_in_stock) {
            neroshop::print("[error]: order failed  [reason]: The following item is out of stock: " + item_name);
            // set order status to failed
            set_status(order_status::failed);
            DB::Postgres::get_singleton()->execute_params("UPDATE orders SET status = $1 WHERE id = $2", { get_status_string(), std::to_string(order_id) });
            // revert everything to the point before we created the order
            neroshop::print("reverting order creation ...", 3);
            DB::Postgres::get_singleton()->execute("ROLLBACK;");
            //DB::Postgres::get_singleton()->finish();
            return; // exit function //continue; // skip this item
        }
        // add each item to the same order_id
        DB::Postgres::get_singleton()->execute_params("INSERT INTO order_item (order_id, item_id, seller_id, item_qty, item_price) "
            "VALUES ($1, $2, $3, $4, $5)", { std::to_string(order_id), std::to_string(item_id), std::to_string(seller_id), std::to_string(item_qty), std::to_string(item_price) });
        // reduce stock_qty of each purchased item (subtract stock_qty by item_qty that buyer is purchasing)
        int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });//std::cout << "stock_qty of item BEFORE deletion: " << stock_qty << std::endl; 
        DB::Postgres::get_singleton()->execute_params("UPDATE inventory SET stock_qty = $1 WHERE item_id = $2 AND seller_id = $3", { std::to_string(stock_qty - item_qty), std::to_string(item_id), std::to_string(seller_id) });//std::cout << "stock_qty of item AFTER deletion: " << db.get_column_integer("inventory", "stock_qty", "item_id = " + std::to_string(item->get_id()) + " AND seller_id = " + std::to_string(seller_id)) << std::endl;
        // If stock qty goes to 0, delete this item from inventory (row) - bad idea because once an item is listed by a seller then the listing is permanent unless the seller's account is deleted or the manufacture of the item has discontinued
        //stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND seller_id = $2", { std::to_string(item_id), std::to_string(seller_id) });
        //if(stock_qty <= 0) DB::Postgres::get_singleton()->execute_params("DELETE FROM inventory WHERE stock_qty = $1", { std::to_string(0) });
    }
    // print order message
    neroshop::print("Thank you for using neroshop.");
    neroshop::io_write("You have ordered: ");
    for(int i = 0; i < cart.get_contents_count(); i++) {
        neroshop::Item * item = cart.get_item(i);
        std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item->get_id()) });//unsigned int item_qty = db.get_column_integer("Cart", "item_qty", "item_id = " + std::to_string(item_id));
        int item_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT item_qty FROM cart_item WHERE cart_id = $1 AND item_id = $2", { std::to_string(cart.get_id()), std::to_string(item->get_id()) });
        std::cout << ((i > 0) ? "                              " : "") << "\033[0;94m" + item_name << " (x" << item_qty << ")\033[0m" << std::endl;
        DB::Postgres::get_singleton()->execute_params("UPDATE cart_item SET item_qty = $1 WHERE cart_id = $2 AND item_id = $3", { std::to_string(0), std::to_string(cart.get_id()), std::to_string(item->get_id()) }); // reset all item quantity to 0 (now that order has been completed)
    }
    // empty cart after completing order
    const_cast<neroshop::Cart&>(cart).empty(user_id);
    // if a user loses internet connection, libcurl cannot get the exchange rate so the order will fail
    // todo: find a way to check if user has internet connection
    this->set_status(order_status::failed); // set order status to failed by default
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET status = $1 WHERE id = $2", { get_status_string(), std::to_string(order_id) });
    // update order details - converts seller's currency of choice to xmr, the moment you create an order
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET subtotal = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(subtotal, seller_currency,  true)), std::to_string(order_id) });
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET discount = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(discount, seller_currency,  true)), std::to_string(order_id) });
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET shipping_cost = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true)), std::to_string(order_id) });
    double total = (subtotal - discount) + shipping_cost;
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET total = $1 WHERE id = $2", { std::to_string(neroshop::Converter::convert_xmr(total, seller_currency,  true)), std::to_string(order_id) });
    // display order details
    std::string currency = Script::get_string(neroshop::get_lua_state(), "neroshop.currency");
    if(currency.empty() || !neroshop::Converter::is_supported_currency(currency)) currency = "usd"; // default //neroshop::Converter::from_xmr(neroshop::Converter::to_xmr(, seller_currency), currency);
    neroshop::print("Sit tight as we notify the seller(s) about your order.");
    std::cout << "Subtotal: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(subtotal, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(subtotal, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl; // (() ? : "")
    if(discount > 0) std::cout << "Discount: -" << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(discount, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (-" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(discount, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    std::cout << "Shipping: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(shipping_cost, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    std::cout << "Order total: " << std::fixed << std::setprecision(12) << neroshop::Converter::convert_xmr(total, seller_currency,  true) << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_symbol(currency) << neroshop::Converter::convert_xmr(neroshop::Converter::convert_xmr(total, seller_currency,  true), currency, false) << " " << String::upper(currency) << ")" <<  std::endl;
    //std::cout << "Estimated delivery date: " << delivery_date_est << std::endl;
    // set order status => pending
    this->set_status(order_status::pending); // if everything went well then order will be set to pending
    DB::Postgres::get_singleton()->execute_params("UPDATE orders SET status = $1 WHERE id = $2", { get_status_string(), std::to_string(order_id) });    
    ////////////////////////////////*/
    // end transaction
    DB::Postgres::get_singleton()->execute("COMMIT;");
    ////////////////////////////////
#endif    
}
////////////////////
void neroshop::Order::cancel_order()
{
    // cannot cancel order if it has been at least 12 hours or more
    // sellers can request that a buyer cancels an order
    // only a buyer can cancel an order
    set_status( order_status::cancelled );
}
////////////////////
void neroshop::Order::change_order()
{}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
void neroshop::Order::set_status(order_status status) { this->status = status;}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
unsigned int neroshop::Order::get_id() const {
    return id;
}
////////////////////
order_status neroshop::Order::get_status() const {return status;}
////////////////////
std::string neroshop::Order::get_status_string() const {
    switch(status) {
        case order_status::incomplete: return "Incomplete";break; // order was interrupted while user was in the process of creating an order
        case order_status::pending   : return "Pending"  ; break;
        case order_status::preparing : return "Preparing"; break;
        case order_status::shipped   : return "Shipped"  ; break;
        case order_status::ready     : return "Ready"    ; break;
        case order_status::done      : return "Delivered"; break;
        case order_status::cancelled : return "Cancelled"; break;
        case order_status::failed    : return "Failed"   ; break;
        case order_status::returned  : return "Returned" ; break;
        //case order_status:: : break;
        default: return "";
    }
}
// if(order->get_status_string() == "pending") {}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
bool neroshop::Order::is_cancelled() const {return (status == order_status::cancelled);}
////////////////////
////////////////////
bool neroshop::Order::in_db(unsigned int order_number) // static - can be called without an obj
{
    /*neroshop::DB::SQLite3 db("neroshop.db");
    int order_id = db.get_column_integer("orders", "id", "id=" + std::to_string(order_number));
    if(order_id <= 0) {std::cout << "Order not found" << std::endl;return false;}
    if(order_id == order_number) {
        return true;
    }
    db.close();*/
    return false;
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
