// filename: .hpp
//#pragma once // use #ifndef _HPP, #define _HPP, and #endif instead for portability

#ifndef ORDER_HPP_NEROSHOP // recommended to add unique identifier like _NEROSHOP to avoid naming collision with other libraries
#define ORDER_HPP_NEROSHOP

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
	static bool in_db(unsigned int order_number); // check if an order is in the order database  // order #144
	unsigned int id;
	order_status status; // order_status
	// invoice: seller_username:layter, seller_receiver_subaddress:Axxxxxx, tx_status:processing(yellow),complete(green[paid]), confirmations:#, product_name:"Playstation 5" total_price:#xmr(#usd), currency:xmr, exchange_rate:1xmr=???usd, created_timestamp:YYYY-MM-DD HH:MM:SS, tx_speed:low,med,high shipping_to:10 rocket st boston ma 02115, tracking_number:####, carrier:usps, delivery_status:shipping_now|out_for_delivery|delivered|
	// options:cancel_order|change_shipping_address,
	void set_status(order_status status);
};
}
#endif
// https://support.bigcommerce.com/s/article/Orders?language=en_US
// https://docs.woocommerce.com/document/managing-orders/
/*
order_status:
    pending   // order created, but on hold/awaiting payment (unpaid) (yellow)
    preparing // order received and now preparing item for shipment (green)
    shipped   // order shipped (can now be tracked and invoice can be printed)
    ready     // order ready for pick up (blue)
    done      // order delivered or picked_up (blue)
    // in any event, an order can be cancelled or can failed ...
    // user must specify reaason why order was cancelled
    cancelled // order cancelled by buyer (can cancel order only before it ships) (gray)
    failed    // order failed due to payment not sent after 24 hours (red)
    returned  // order returned (returned does not necessarily mean refunded) (gray)

	// just making sure enum contents are the same value
	std::cout << "(order_status::ready == order_status::ready_for_pickup) : " << (order_status::ready == order_status::ready_for_pickup) << std::endl;
	std::cout << "(order_status::done == order_status::delivered) : " << (order_status::done == order_status::delivered) << std::endl;

Categories:
Carriers: usps, fedex, ups, dhl (make a class Tracker for the carrier)
          https://instantparcels.com/find-carrier-by-tracking-number
          https://developers.facebook.com/docs/commerce-platform/order-management/carrier-codes
Payment methods: cryptocurrency (monero)
// NOTE: for every order, generate a new subaddress from seller's wallet
// order_cancellations: https://sellercentral.amazon.com/gp/help/external/G201722390?language=en_US
*/
