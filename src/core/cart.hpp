//#pragma once
#ifndef CART_HPP_NEROSHOP
#define CART_HPP_NEROSHOP

#include <iostream>
#include <memory> // std::shared_ptr, std::unique_ptr
#include <vector>
#include <algorithm> // std::find
#include <map> // std::map

#include "product.hpp"

namespace neroshop {

enum class CartError {
    Ok = 0,
    Missing,
    Full,
    ItemOutOfStock,
    ItemQuantitySurpassed,
    ItemQuantityNotSpecified,
    SellerAddOwnItem,
};

class Cart {
public:
    Cart();
    ~Cart();
    CartError add(const std::string& user_id, const std::string& listing_key, int quantity = 1);
    void add(const std::string& user_id, const neroshop::Product& item, int quantity = 1);
    void remove(const std::string& user_id, const std::string& listing_key, int quantity = 1);
    void remove(const std::string& user_id, const neroshop::Product& item, int quantity = 1);

    void empty(); // remove all items from cart
    void change_quantity(const std::string& user_id, const neroshop::Product& item, int quantity); // set_quantity is private so you can only change item quantity from this function
    //void move_to_wishlist();
    //void save_for_later();
    //void shift_up(const neroshop::Product& item);
    //void shift_down(const neroshop::Product& item);
	//void swap_positions(const neroshop::Product& item1, const neroshop::Product& item2);
	//void checkout(); // user's cart contents impact inventory availability. Only after purchase will actual inventory decrease
	void print_cart();
	// setters
	// getters
	static int get_max_items(); // returns the maximum number of items (contents) a cart can hold
	static int get_max_quantity(); // returns the maximum quantity of items a cart can hold	
	int get_quantity() const; // returns the sum of all item quantity within a cart
	
	double get_seller_subtotal_price(unsigned int seller_id = 0) const;
	double get_subtotal_price() const; // This is the total price for each product in the cart (combined)
	//double get_total_price() const; // This is the total price for all products in cart (combined with shipping and taxes)
	unsigned int get_total_quantity() const; // same as get_items_count; returns the total cart quantity	
	static unsigned int get_total_quantity_db();
	double get_total_weight() const; // returns total weight of all items in cart
	double get_seller_total_discount(unsigned int seller_id = 0) const;
	double get_total_discount() const; // coupons can be applied while item is in cart
	int get_items_count() const; // number of items in the cart based on quantity
	int get_contents_count() const; // returns number of items in cart.contents
	neroshop::Product * get_item(unsigned int index) const; //unsigned int get_id() const;//static std::string get_file();
	std::vector<std::shared_ptr<neroshop::Product>> get_contents_list() const;
    // getters - user
    std::string get_id() const;
	std::string get_owner_id() const; // returns the id of the user who owns this cart
	static unsigned int get_owner_id(unsigned int cart_id); // returns the id of the user who owns this cart	
	std::size_t get_listing_index(const std::string& listing_key);
	// boolean
	bool is_empty() const;
    bool is_full() const; // cart is full (has reached max items)
	bool in_cart(const std::string& listing_key) const;
	bool in_cart(const neroshop::Product& item) const;
	//bool validate_item(const neroshop::Product& item) const;
	// friends - can access cart's private members
	friend class User;
	friend class Buyer;
	friend class Seller;
	friend class Order;
	friend class Serializer;
private:
    std::string id;
    std::string owner_id;
    std::vector<std::tuple<std::string, int, std::string>> contents;
    static unsigned int max_items; // cart can only hold up to 10 unique items
    static unsigned int max_quantity; // the max quantity each item can add up to is 100, so 10 items can each have a quantity of 10, making the total number of items 100
    void load(const std::string& user_id); // loads cart data in-memory (called on user login)
    void set_id(const std::string& id);
    void set_owner_id(const std::string& owner_id);
    void set_contents(const std::vector<std::tuple<std::string, int, std::string>>& contents);
};
}
#endif
