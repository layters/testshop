#pragma once

#ifndef USER_HPP_NEROSHOP
#define USER_HPP_NEROSHOP

#include <iostream>
#include <memory> // std::unique_ptr
#include <string>
#include <vector>
#include <utility> // std::pair

#include "product.hpp"
#include "order.hpp"

enum class UserAccountType : unsigned int { Guest, Buyer, Seller };

namespace neroshop {

class Wallet;
class Cart; // forward declaration

class User { // base class of seller and buyer // sellers can buy and sell while buyers (including guests) can only buy but cannot sell
public: 
    User();
    virtual ~User(); // by making this virtual both base and derived class destructors will be called instead of just the base destructor alone
    // account-related stuff (will continue adding more account features)
    void rate_seller(const std::string& seller_id, int score, const std::string& comments, const std::string& signature); // seller score (0-1) // use int and NOT unsigned int 'cause unsigned int assumes the arg will never be negative number, but when arg is negative, it converts it to some random positive number
    void rate_item(const std::string& item_id, int stars, const std::string& comments, const std::string& signature); // star ratings (1-5)    
    //void report_user(const User& user, const std::string& reason); // report a user
    void delete_account();
    void logout();
    // cart-related stuff (50% complete - cart class still needs some more work)
    int add_to_cart(const std::string& listing_key, int quantity = 1);
    void remove_from_cart(const std::string& listing_key, int quantity = 1);
    void clear_cart();
    // order-related stuff (50% complete - order class still needs some more work)
    void create_order(const std::string& shipping_address);// const;//void create_order();
    // favorite-or-wishlist-related stuff (100% complete)
    void add_to_favorites(const std::string& listing_key);
    void remove_from_favorites(const std::string& listing_key);
    void clear_favorites();
    // avatar-related stuff (10% complete)
    void upload_avatar(const std::string& filename);
    void delete_avatar();
    // private messages
    void send_message(const std::string& recipient_id, const std::string& content, const std::string& recipient_public_key);
    std::pair<std::string, std::string> decrypt_message(const std::string& content_encoded, const std::string& sender_encoded);
    // setters
    void set_public_key(const std::string& public_key);
    void set_private_key(const std::string& private_key);
	// setters - wallet-related stuff
	void set_wallet(const neroshop::Wallet* wallet);
    // getters
    std::string get_public_key() const;
	// getters - wallet-related stuff
	neroshop::Wallet * get_wallet() const;
    // account-related stuff - getters
    std::string get_id() const;//unsigned int get_id() const;
    std::string get_name() const;
    UserAccountType get_account_type() const;
    std::string get_account_type_string() const;
    Image * get_avatar() const;
    int get_account_age() const; // Returns account age (in days)
    static int get_account_age(const std::string& user_id);
    // buyer-related stuff - getters
    neroshop::Cart * get_cart() const;
    neroshop::Order * get_order(unsigned int index) const;
    unsigned int get_order_count() const;
    std::vector<neroshop::Order *> get_order_list() const;
    std::string get_favorite(unsigned int index) const;
    unsigned int get_favorites_count() const;
    std::vector<std::string> get_favorites() const;
    // boolean
    bool is_guest() const;
    bool is_buyer() const;
    bool is_seller() const;
    bool is_online() const; // online does not mean logged in // checks if user has internet, and user is logged_in
    bool is_registered() const;
    static bool is_registered(const std::string& name);
    bool is_logged() const; // the same for every derived class // user has entered their login information
    bool has_email() const;
    bool has_avatar() const;
    // item-related stuff - boolean
    bool has_purchased(const std::string& listing_key); // checks if an item was previously purchased or not
    bool has_favorited(const std::string& listing_key); // checks if an item is in a user's favorites or wishlist
	bool has_wallet() const; // returns true if seller's wallet is opened
	bool has_wallet_synced() const; // returns true if seller's wallet is synced to a node
    // callbacks
    void on_registration(const std::string& name); // on registering an account
    //virtual User * on_login(const std::string& username);// = 0; // load all data: orders, reputation/ratings, settings // for all users
    void on_checkout();//(const neroshop::Order& order); // for all users
    virtual void on_order_received(); // for sellers only
    // friends
    friend class Backend;
    friend class UserManager;
    friend class Serializer;
    friend class Seller;
    void set_id(const std::string& id);
    void set_name(const std::string& name); // the same for every derived class
protected: // can only be accessed by classes that inherit from class User (even instants of the bass class User cannot call these functions unless you dynamically cast them into a derived class)
    void set_account_type(UserAccountType account_type); // either buyer or seller // the same for every derived class 
    void set_logged(bool logged); // the same for every derived class
    void set_online(bool online);
    // loading into memory so we don't always have to fetch from the database within the same session
    void load_cart();
    void load_orders(); // on login, load all orders this user has made so far (this function is called only once per login)
    void load_favorites(); // on login, load favorites (this function is called only once per login)
	// Make wallet a non-owning raw pointer or maybe use std::weak_ptr if shared ownership is needed
	neroshop::Wallet* wallet;
private:
    std::string id;
    std::string name;
    UserAccountType account_type; // seller, buyer (guest)
    bool logged; // determines whether user is logged in or not//bool online;
    std::string public_key;
    std::string private_key;
    std::unique_ptr<Cart> cart;
    std::vector<std::shared_ptr<neroshop::Order>> order_list;
    std::vector<std::string> favorites;
    std::string get_private_key() const;
    std::unique_ptr<Image> avatar;
};
}
#endif
