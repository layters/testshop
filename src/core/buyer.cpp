#include "buyer.hpp"

#include "util.hpp" // neroshop::string::lower

////////////////////
neroshop::Buyer::Buyer() {} // not used
////////////////////
neroshop::Buyer::Buyer(const std::string& name) : Buyer() {
    set_name(name);
    //std::cout << "buyer created\n";
}
////////////////////
neroshop::Buyer::~Buyer() {
#ifdef NEROSHOP_DEBUG0
    std::cout << "buyer deleted\n";
#endif    
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
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
neroshop::User * neroshop::Buyer::on_login(const std::string& username) { // if validator::login(user, pw) returns true, then set User::logged to true
    // retrieve user data from database
    /*DB db("neroshop.db");
    neroshop::User * user = new Buyer(username); // create seller obj
    dynamic_cast<Buyer *>(user)->set_logged(true); // protected, so only instance of derived class can call this function
    dynamic_cast<Buyer *>(user)->set_id(db.get_column_integer("users", "id", "name = " + DB::to_sql_string(username)));
    dynamic_cast<Buyer *>(user)->set_account_type(user_account_type::buyer); // set the account_type
#ifdef NEROSHOP_DEBUG
        std::cout << "\033[1;34m(account_type: " << neroshop::string::lower(user->get_account_type_string()) << ", id: " << user->get_id() << ")\033[0m" << std::endl;
#endif    
    ///////////
    // load orders
    dynamic_cast<Buyer *>(user)->load_orders();
    // load wishlists
    // ...
    db.close();*/ // don't forget to close the db when done!
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    neroshop::User * user = new Buyer(username); // create seller obj
    dynamic_cast<Buyer *>(user)->set_logged(true); // protected, so only instance of derived class can call this function
#if defined(NEROSHOP_USE_POSTGRESQL)     
    int user_id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM users WHERE name = $1", { username });
    dynamic_cast<Buyer *>(user)->set_id(user_id);
#endif    
    dynamic_cast<Buyer *>(user)->set_account_type(user_account_type::buyer); // set the account_type    
    //-----------------------------------------
    //DB::Postgres::get_singleton()->finish();
    // save user to global static object for easy access
    //User::set_singleton(*user);
    //-----------------------------------------
    // load orders
    dynamic_cast<Buyer *>(user)->load_orders();
    // load wishlists
    dynamic_cast<Buyer *>(user)->load_favorites();
    // load cart (into memory)
    ////if(user->is_registered()) {
        ////user->get_cart()->load_cart(user->get_id());//Cart::get_singleton()->load_cart(user->get_id());
        //std::cout << "subtotal_price: " << Cart::get_singleton()->get_subtotal_price(user->get_id()) << std::endl;
        //std::cout << "total_weight: " << Cart::get_singleton()->get_total_weight(user->get_id()) << std::endl;
    ////}
    ////////////////////////////////
#ifdef NEROSHOP_DEBUG
        std::cout << "\033[1;34m(account_type: " << neroshop::string::lower(user->get_account_type_string()) << ", id: " << user->get_id() << ")\033[0m" << std::endl;
#endif        
    ////////////////////////////////    
    return user;
    // static_cast to convert from User to Buyer (static_cast<Buyer *>(user))
    // dynamic_cast to use a User as if it were a Buyer (dynamic_cast<Sprite *>(entity)->is_sprite())
}
////////////////////
////////////////////
