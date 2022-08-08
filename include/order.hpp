#pragma once

#ifndef ORDER_HPP_NEROSHOP
#define ORDER_HPP_NEROSHOP

#if defined(__cplusplus) && (__cplusplus >= 201703L)
#include <uuid.h>
//#include <catch.hpp>
#endif

#include <iostream>
#include <map>

//#include "wallet.hpp"
#include "cart.hpp"      // includes db.hpp
#include "converter.hpp" // currency converter
#include "config.hpp" // neroshop::lua_state

enum class payment_status{ PAYMENT_NOT_RECEIVED, // red // https://stackoverflow.com/a/46740323
    PAYMENT_CONFIRMED,    // yellow
    PAYMENT_RECEIVED,     // green
};
enum class order_status : unsigned int {incomplete, created, pending = created, preparing, shipped, ready_for_pickup, ready = ready_for_pickup, delivered, done = delivered, cancelled, failed, returned,}; // char, short or unsigned int // enum classes help avoid polluting the scope (either global or namespace)
enum class payment_method{/*cash, card,*/ crypto,};
enum class currency{xmr,};

namespace neroshop {
class Order { // create a db for orders that stores order numbers and details and retrieve order
public:
	Order();
	Order(unsigned int id);
	~Order();
	void create_order(const neroshop::Cart& cart, const std::string& shipping_address, std::string contact_info = ""); // order: order_id, [order_date], product, SKU, quantity, price (subtotal), discount (optional), shipping_cost/estimated_delivery, carrier[dhl, usps, etc.], payment method:monero[xmr], total
	void cancel_order(); // revoke the order
	void change_order(); // edit the order info such as: shipping_address, contact_info, removing individual items from order or item_qty, or adding a note, applying coupon
	void download_order(); // order details will be in JSON format
	//void print_order();//print_invoice(); // will require a printer
	// order_item-related functions
	void cancel_order_item(); // remove an item from a order
	void change_order_item(); // replace order_item with a different item
	void change_order_item_quantity(); // change order_item quantity
	// setters
	// getters
	unsigned int get_id() const;
	order_status get_status() const;
	std::string get_status_string() const; // pending (awaiting_payment), processing (already paid for, now seller is preparing), preparing, shipped (fully | partially), refunded or returned (fully | partially), declined, disputed, failed (user fails to pay), cancelled, done (or completed; order was paid for and delivered to the buyer)
	// boolean
	bool is_cancelled() const;
	// friend
	//friend class Buyer; // order can now access buyer's private functions
	friend class Seller; // seller can edit order_status, except cancellation, done,
private:
	void create_guest_order(const neroshop::Cart& cart, const std::string& shipping_address, std::string contact_info = ""); // order: order_id, [order_date], product, SKU, quantity, price (subtotal), discount (optional), shipping_cost/estimated_delivery, carrier[dhl, usps, etc.], payment method:monero[xmr], total
	void create_registered_user_order(const neroshop::Cart& cart, const std::string& shipping_address, std::string contact_info = "");
	static bool in_db(unsigned int order_number); // check if an order is in the order database
	unsigned int id;
	order_status status;
	void set_status(order_status status);
};
}
#endif
