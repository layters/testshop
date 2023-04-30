#pragma once

#ifndef ORDER_HPP_NEROSHOP
#define ORDER_HPP_NEROSHOP

#include <iostream>
#include <string>
#include <vector>

namespace neroshop {

class Cart; // forward declaration

// order status
enum class OrderStatus : unsigned int {
    Order_Incomplete, 
    Order_Created, 
    Order_Pending = Order_Created, 
    Order_Preparing, 
    Order_Shipped, 
    Order_ReadyForPickup, 
    Order_Ready = Order_ReadyForPickup, 
    Order_Delivered, 
    Order_Done = Order_Delivered, 
    Order_Cancelled, 
    Order_Failed, 
    Order_Returned,    
};

// payment status
enum class PaymentStatus {
    Payment_NotReceived, // red
    Payment_Confirmed, // yellow
    Payment_Received, // green
};

// payment options
enum class PaymentOption {
    Escrow = 0, // 2 of 3
    Multisig, // 2 of 2
    Finalize, // no escrow (Note: all cash and card payments are finalize by default)
};

// payment methods - may not be necessary since only crypto will be used
enum class PaymentMethod {
    Cash, 
    Card, // can be either credit, debit, or pre-paid
    Crypto, 
    Metal, // can be any precious metal
};

// payment coins (cryptocurrencies used for payments)
enum class PaymentCoin { 
    None = -1, // intended for cash and card payments
    Monero,
    //Wownero, // coming soon!
    //Bitcoin, // coming soon or not :S
};

// delivery options
enum class DeliveryOption {
    Delivery = 0,
    Pickup,
};

// delivery methods
enum class DeliveryMethod {
    Mail = 0,
    Courier,
    Digitally,
};

class Order { 
public:
    Order();
	Order(const std::string& id, const std::string& date, OrderStatus status, const std::string& customer_id,
	    double subtotal, double discount, double shipping_cost, double total, PaymentOption payment_option,
	    PaymentCoin payment_coin, DeliveryOption delivery_option, const std::string& notes,
	    const std::vector<std::tuple<std::string, int, std::string>>& items);
	~Order();
	void create_order(const neroshop::Cart& cart, const std::string& shipping_address);
	void cancel_order(); // revoke the order
	void change_order(); // edit the order info such as: shipping_address, contact_info, removing individual items from order or item_qty, or adding a note, applying coupon
	void download_order(); // order details will be in JSON format
	// order item-related functions
	void cancel_order_item(); // remove an item from a order
	void change_order_item(); // replace order_item with a different item
	void change_order_item_quantity(); // change order_item quantity
	// setters - there are no setters. Call `create_order` to set the member variable values
	// getters
	std::string get_id() const;
	OrderStatus get_status() const;
	std::string get_status_as_string() const; // pending (awaiting_payment), processing (already paid for, now seller is preparing), preparing, shipped (fully | partially), refunded or returned (fully | partially), declined, disputed, failed (user fails to pay), cancelled, done (or completed; order was paid for and delivered to the buyer)
	std::string get_date() const;
	std::string get_customer_id() const;
	double get_subtotal() const;
	double get_discount() const;
	double get_shipping_cost() const;
	double get_total() const;
	PaymentOption get_payment_option() const;
	std::string get_payment_option_as_string() const;
	PaymentCoin get_payment_coin() const;
	std::string get_payment_coin_as_string() const;
	DeliveryOption get_delivery_option() const;
	std::string get_delivery_option_as_string() const;
	std::string get_notes() const;
	std::vector<std::tuple<std::string, int, std::string>> get_items() const;
	// boolean
	bool is_cancelled() const;
	// friend
	friend class Seller;
private:
	std::string id;
	std::string date; // date of creation
	OrderStatus status;
	std::string customer_id;
	double subtotal;
	double discount;
	double shipping_cost;
	double total;
	PaymentOption payment_option; // "Escrow", "Multisig", "Finalize"
	PaymentCoin payment_coin; // "Monero"
	DeliveryOption delivery_option; // "Delivery", "Pickup"
	std::string notes; // encrypted note containing sensative information
	std::vector<std::tuple<std::string, int, std::string>> items; // I am using vector so that each item is stored in the order by which it is placed/inserted
	// TODO: make cart contents a vector instead of a map?
	void set_status(OrderStatus status);
};

}
#endif
