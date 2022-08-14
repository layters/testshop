//#pragma once
#ifndef CART_HPP_NEROSHOP
#define CART_HPP_NEROSHOP

#include <iostream>
#include <memory> // std::shared_ptr, std::unique_ptr
#include <vector>
#include <algorithm> // std::find

#include "item.hpp"
#include "database.hpp"

namespace neroshop {
class Cart {
public:
    Cart();
    ~Cart();
    
    void add(unsigned int item_id, int quantity = 1);
    void add(const neroshop::Item& item, int quantity = 1);//static void add(unsigned int cart_id, unsigned int item_id, int quantity = 1);
    void remove(unsigned int item_id, int quantity = 1);
    void remove(const neroshop::Item& item, int quantity = 1);//static void remove(unsigned int cart_id, unsigned int item_id, int quantity = 1);

    void empty(); // remove all items from cart
    void change_quantity(const neroshop::Item& item, int quantity); // set_quantity is private so you can only change item quantity from this function
    //void move_to_wishlist();
    //void save_for_later();
    //void shift_up(const neroshop::Item& item);
    //void shift_down(const neroshop::Item& item);
	//void swap_positions(const neroshop::Item& item1, const neroshop::Item& item2);
	//void checkout(); // user's cart contents impact inventory availability. Only after purchase will actual inventory decrease
	// setters
	// getters
	unsigned int get_max_items() const; // returns the maximum number of items (contents) a cart can hold
	unsigned int get_max_quantity() const; // returns the maximum quantity of items a cart can hold	
	
	double get_seller_subtotal_price(unsigned int seller_id = 0) const;
	double get_subtotal_price() const; // This is the total price for each product in the cart (combined)
	//double get_total_price() const; // This is the total price for all products in cart (combined with shipping and taxes)
	unsigned int get_total_quantity() const; // same as get_items_count; returns the total cart quantity	
	static unsigned int get_total_quantity_db();
	double get_total_weight() const; // returns total weight of all items in cart
	double get_seller_total_discount(unsigned int seller_id = 0) const;
	double get_total_discount() const; // coupons can be applied while item is in cart
	unsigned int get_items_count() const; // number of items in the cart based on quantity
	unsigned int get_contents_count() const; // returns number of items in cart.contents
	neroshop::Item * get_item(unsigned int index) const; //unsigned int get_id() const;//static std::string get_file();
	std::vector<std::shared_ptr<neroshop::Item>> get_contents_list() const;
    // getters - user
    unsigned int get_id() const;
	unsigned int get_owner_id() const; // returns the id of the user who owns this cart
	static unsigned int get_owner_id(unsigned int cart_id); // returns the id of the user who owns this cart	
	// boolean
	bool is_empty() const;//bool is_empty(int user_id) const;
    bool is_full() const; // cart is full (has reached max items)
	bool in_cart(unsigned int item_id) const;
	bool in_cart(const neroshop::Item& item) const;
	//bool validate_item(const neroshop::Item& item) const;
	// friends
	friend class User;//Buyer; // buyer can access cart's private members
	friend class Order;
private:
    int id; // 0 by default
    std::vector<std::shared_ptr<neroshop::Item>> contents;//protected: // cannot be accessed outside of class but by a derived class (subclass)
    unsigned int max_items; // cart can only hold up to 10 items
    unsigned int max_quantity; // the max quantity each item can add up to is 100, so 10 items can have 10 quantity each, making the total number of items 100
    void load(const neroshop::Item& item, unsigned int quantity); // loads cart.db on app start
};
}
#endif
