#pragma once

#ifndef ORDER_HPP_NEROSHOP
#define ORDER_HPP_NEROSHOP

#include <iostream>
#include <string>
#include <vector>

#include "payment.hpp"
#include "delivery.hpp"

namespace neroshop {

class Cart; // forward declaration

// order status
enum class OrderStatus {
    New, // Newly created or placed order
    Pending, // Order is approved and is now awaiting payment
    Processing, // Order that is being prepared or assembled
    Shipped, // Order has been shipped
    ReadyForPickup, Ready = ReadyForPickup, // Order is ready for pickup from a seller-specified location
    Delivered, Done = Delivered, // Order that has been delivered or picked up
    Cancelled, // Order was cancelled by the buyer
    Failed, // Order failed due to an error or technical issue
    Returned, // Order has been returned
    Disputed, // Customer has initiated a dispute process
    Declined, // Order was declined by the seller
};

struct OrderItem {
    std::string key;
    unsigned int quantity = 0;
    std::string seller_id;
};

class Order { 
public:
    Order();
	Order(const std::string& id, const std::string& date, OrderStatus status, const std::string& customer_id,
	    double subtotal, double discount, double shipping_cost, double total, PaymentOption payment_option,
	    PaymentCoin payment_coin, DeliveryOption delivery_option, const std::string& notes,
	    const std::vector<OrderItem>& items);
	~Order();
	void create_order(const neroshop::Cart& cart, const std::string& shipping_address);
	void cancel_order(); // revoke the order
	void change_order(); // edit the order info such as: shipping_address, contact_info, removing individual items from order or item_qty, or adding a note, applying coupon
	void download_order(); // order details will be in JSON format
	// order item-related functions
	void cancel_order_item(); // remove an item from a order
	void change_order_item(); // replace order_item with a different item
	void change_order_item_quantity();
	
	std::string get_id() const;
	OrderStatus get_status() const;
	std::string get_status_as_string() const; // pending (awaiting_payment), processing (already paid for, now seller is preparing), preparing, shipped (fully | partially), refunded or returned (fully | partially), declined, disputed, failed (user fails to pay), cancelled, done (or completed; order was paid for and delivered to the buyer)
	std::string get_date() const;
	std::string get_customer_id() const;
	double get_subtotal() const;
	double get_discount() const;
	double get_shipping_cost() const;
	double get_total() const;
	PaymentMethod get_payment_method() const;
	std::string get_payment_method_as_string() const;
	PaymentOption get_payment_option() const;
	std::string get_payment_option_as_string() const;
	PaymentCoin get_payment_coin() const;
	std::string get_payment_coin_as_string() const;
	DeliveryOption get_delivery_option() const;
	std::string get_delivery_option_as_string() const;
	std::string get_notes() const;
	std::vector<OrderItem> get_items() const;
	// boolean
	bool is_cancelled() const;
	// friend
	friend class Seller;
	friend class Serializer;
private:
	std::string id;
	std::string date; // date of creation
	OrderStatus status;
	std::string customer_id;
	double subtotal;
	double discount;
	double shipping_cost;
	double total;
	PaymentMethod payment_method;
	PaymentOption payment_option; // "Escrow", "Multisig", "Finalize"
	PaymentCoin payment_coin; // "Monero"
	DeliveryOption delivery_option; // "Delivery", "Pickup"
	std::string notes; // encrypted note containing sensative information
	std::vector<OrderItem> items; // <product_id>,<quantity>,<seller_id>
	// TODO: make cart contents a vector instead of a map so cart items can be in correct order
	void set_id(const std::string& id);
	void set_date(const std::string& date);
	void set_status(OrderStatus status);
	void set_status_by_string(const std::string& status);
	void set_customer_id(const std::string& customer_id);
	void set_subtotal(double subtotal);
	void set_discount(double discount);
	void set_shipping_cost(double shipping_cost);
	void set_total(double total);
	void set_payment_method(PaymentMethod payment_method);
    void set_payment_method_by_string(const std::string& payment_method);
	void set_payment_option(PaymentOption payment_option);
	void set_payment_option_by_string(const std::string& payment_option);
	void set_payment_coin(PaymentCoin payment_coin);
	void set_payment_coin_by_string(const std::string& payment_coin);
	void set_delivery_option(DeliveryOption delivery_option);
	void set_delivery_option_by_string(const std::string& delivery_option);
	void set_notes(const std::string& notes);		
	void set_items(const std::vector<OrderItem>& items);
	void create_order_batch(const neroshop::Cart& cart, const std::string& shipping_address);
};

}
#endif
