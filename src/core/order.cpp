#include "order.hpp"

#include "cart.hpp"
#include "price/currency_converter.hpp" // currency converter, enums.hpp
#include "config.hpp" // neroshop::lua_state
#include "database/database.hpp"
#include "tools/logger.hpp"
#include "tools/tools.hpp" // neroshop::uuid::generate()

////////////////////
neroshop::Order::Order() : status(OrderStatus::New), subtotal(0.00), discount(0.00), shipping_cost(0.00), total(0.00), 
    payment_option(PaymentOption::Escrow), payment_coin(PaymentCoin::Monero), delivery_option(DeliveryOption::Delivery)
{}

neroshop::Order::Order(const std::string& id, const std::string& date, OrderStatus status, const std::string& customer_id,
	    double subtotal, double discount, double shipping_cost, double total, PaymentOption payment_option,
	    PaymentCoin payment_coin, DeliveryOption delivery_option, const std::string& notes,
	    const std::vector<std::tuple<std::string, int, std::string>>& items)
	: id(id), date(date), status(status), customer_id(customer_id), subtotal(subtotal), discount(discount),
	  shipping_cost(shipping_cost), total(total), payment_option(payment_option), payment_coin(payment_coin),
	  delivery_option(delivery_option), notes(notes), items(items)
{}

neroshop::Order::~Order() {
#ifdef NEROSHOP_DEBUG
    std::cout << "order deleted\n";
#endif    
}
////////////////////
////////////////////
void neroshop::Order::create_order(const neroshop::Cart& cart, const std::string& shipping_address) {
    neroshop::db::Sqlite3 * database = neroshop::get_database();
    if(!database) throw std::runtime_error("database is NULL");

    std::string customer_id = database->get_text_params("SELECT user_id FROM cart WHERE uuid = $1;", { cart.get_id() });
    std::cout << "cart belongs to user_id: " << customer_id << std::endl;
        
    if(cart.is_empty()) { neroshop::print("You cannot place an order because your cart is empty", 1); return; }

    database->execute("BEGIN;"); // begin transaction
    //-------------------------------------------------------
    // Create an incomplete order
    std::string order_id = database->get_text_params("INSERT INTO orders (uuid, customer_id, status, subtotal, discount, shipping_cost, total, payment_option, coin, notes) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10) RETURNING uuid", 
        { neroshop::uuid::generate(), 
        customer_id, // customer or buyer id
        get_status_as_string(), // order status
        // TODO: amount will be stored in piconeros
        std::to_string(0), // subtotal
        std::to_string(0), // discount
        std::to_string(0), // shipping_cost
        std::to_string(0), // total
        "escrow", // payment_option
        "monero", // coin
        shipping_address }); // notes?
        // TODO place monetary amounts, payment_option, coin, additional notes, shipping address, etc. in a JSON string as column "order_data"
        // TODO content in column "order_data" MUST be encrypted
    if(order_id.empty()) { neroshop::print("Order creation failed", 1); database->execute("ROLLBACK;"); return; }
    this->id = order_id;
    //-------------------------------------------------------
    double subtotal = 0.00, discount = 0.00, shipping_cost = 0.00, total = 0.00;
    std::string seller_currency = "USD";
    for (const auto& [item_id, item_qty, seller_id] : cart.contents) {
        std::string item_name = database->get_text_params("SELECT name FROM products WHERE uuid = $1", { item_id });
        
        // If seller_id is not specified, then choose a random seller who is selling the same product, but it MUST be in stock!!
        ////std::string seller_id = database->get_text_params("SELECT seller_id FROM listings WHERE product_id = $1 AND quantity > 0", { item_id });
        if(seller_id.empty()) { std::cout << "item seller not found" << std::endl; database->execute("ROLLBACK;"); return; }//database->finish(); return; }
        // Prevent customer from purchasing their own products
        if(customer_id == seller_id) { neroshop::print("You cannot purchase your own product(s)", 1); database->execute("ROLLBACK;"); return; }
        
        // Get currency that product is priced in
        seller_currency = database->get_text_params("SELECT currency FROM listings WHERE product_id = $1 AND seller_id = $2 AND quantity > 0", { item_id, seller_id });
        
        // Get sales and/or retail price
        double sales_price = database->get_real_params("SELECT price FROM listings WHERE product_id = $1 AND seller_id = $2", { item_id, seller_id });//NEROSHOP_TAG_OUT std::cout << "Seller's price for item (id: " << item_id << ") per unit is: " << sales_price << " " << seller_currency << std::endl;
        ////double unit_price = (sales_price > 0.00) ? sales_price : item->get_price(); // set the retail price to the sales price
    
        // Calculate the subtotal (price of all items combined)
        subtotal += item_qty * sales_price;
        
        // Deal with the discount later ...
        
        // Check again to see if item is still in stock
        bool item_in_stock = database->get_integer_params("SELECT EXISTS(SELECT quantity FROM listings WHERE product_id = $1 AND quantity > 0 AND seller_id = $2)", { item_id, seller_id });
        if(!item_in_stock) {
            neroshop::print("Order failed (Reason: The following item is out of stock: " + item_name + ")");
            // Set order status to failed
            set_status(OrderStatus::Failed);
            database->execute_params("UPDATE orders SET status = $1 WHERE uuid = $2", { get_status_as_string(), order_id });
            database->execute("ROLLBACK;");
            return;
        }
        // Add each product to order_item of the same order_id
        database->execute_params("INSERT INTO order_item (order_id, product_id, seller_id, quantity) "//, item_price) "
            "VALUES ($1, $2, $3, $4)"/*, $5)"*/, { order_id, item_id, seller_id, std::to_string(item_qty)/*, std::to_string(item_price)*/ });
        // Reduce stock quantity of each purchased item
        int stock_qty = database->get_integer_params("SELECT quantity FROM listings WHERE product_id = $1 AND seller_id = $2", { item_id, seller_id });std::cout << "quantity of item BEFORE deletion: " << stock_qty << std::endl; 
        database->execute_params("UPDATE listings SET quantity = $1 WHERE product_id = $2 AND seller_id = $3", { std::to_string(stock_qty - item_qty), item_id, seller_id });std::cout << "quantity of item AFTER deletion: " << database->get_integer_params("SELECT quantity FROM listings WHERE product_id = $1 AND seller_id = $2", { item_id, seller_id }) << std::endl;
    }
    //-------------------------------------------------------
    // Print order message
    neroshop::print("Thank you for using neroshop.");
    neroshop::io_write("You have ordered: ");
    for (const auto& [item_id, item_qty, seller_id] : cart.contents) {
        std::string item_name = database->get_text_params("SELECT name FROM products WHERE uuid = $1", { item_id });
        ////int item_qty = database->get_integer_params("SELECT quantity FROM cart_item WHERE cart_id = $1 AND product_id = $2", { cart.get_id(), item_id });
        std::cout << "\033[0;94m" + item_name << " (x" << item_qty << ")\033[0m" << std::endl;
        database->execute_params("UPDATE cart_item SET quantity = $1 WHERE cart_id = $2 AND product_id = $3", { std::to_string(0), cart.get_id(), item_id }); // Reset all cart_item quantity to 0 (now that order has been completed)
    }
    // Empty cart after completing order
    const_cast<neroshop::Cart&>(cart).empty();
    //-------------------------------------------------------
    // TODO: Update order in first loop because seller_currency is different for each seller
    set_status(OrderStatus::Failed); // Update order status to "failed"
    database->execute_params("UPDATE orders SET status = $1 WHERE uuid = $2", { get_status_as_string(), order_id });
    // TODO: Store price in database as piconeros
    // Convert price to Monero (XMR)
    double subtotal_monero = neroshop::Converter::convert_to_xmr(subtotal, seller_currency);
    double discount_monero = neroshop::Converter::convert_to_xmr(discount, seller_currency);
    double shipping_cost_monero = neroshop::Converter::convert_to_xmr(shipping_cost, seller_currency);
    total = (subtotal - discount) + shipping_cost;
    double total_monero = neroshop::Converter::convert_to_xmr(total, seller_currency);
    // Convert monero to piconero (for storing in database)
    double piconero = 0.000000000001;
    uint64_t subtotal_piconero = subtotal_monero / piconero;
    uint64_t discount_piconero = discount_monero / piconero;
    uint64_t shipping_cost_piconero = shipping_cost_monero / piconero;
    uint64_t total_piconero = total_monero / piconero;
    // Update order details - converts seller's currency of choice to xmr, the moment you create an order
    database->execute_params("UPDATE orders SET subtotal = $1 WHERE uuid = $2", { std::to_string(subtotal_piconero), order_id });
    database->execute_params("UPDATE orders SET discount = $1 WHERE uuid = $2", { std::to_string(discount_piconero), order_id });
    database->execute_params("UPDATE orders SET shipping_cost = $1 WHERE uuid = $2", { std::to_string(shipping_cost_piconero), order_id });
    database->execute_params("UPDATE orders SET total = $1 WHERE uuid = $2", { std::to_string(total_piconero), order_id });
    // Display order details
    std::string your_currency = Script::get_string(neroshop::get_lua_state(), "neroshop.generalsettings.currency");
    if(your_currency.empty() || !neroshop::Converter::is_supported_currency(your_currency)) your_currency = "USD"; // default //neroshop::Converter::from_xmr(neroshop::Converter::to_xmr(, seller_currency), your_currency);
    neroshop::print("Sit tight as we notify the seller(s) about your order.");
    auto from = Converter::get_currency_enum(seller_currency);
    auto to = Converter::get_currency_enum(your_currency);
    std::cout << "Subtotal: " << std::fixed << std::setprecision(12) << subtotal_monero << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_sign(your_currency) << (neroshop::Converter::get_price(from, to) * subtotal) << " " << neroshop::string::upper(your_currency) << ")" <<  std::endl; // (() ? : "")
    if(discount > 0) std::cout << "Discount: -" << std::fixed << std::setprecision(12) << discount_monero << " xmr" << std::fixed << std::setprecision(2) << " (-" << neroshop::Converter::get_currency_sign(your_currency) << (neroshop::Converter::get_price(from, to) * discount) << " " << neroshop::string::upper(your_currency) << ")" <<  std::endl;
    std::cout << "Shipping: " << std::fixed << std::setprecision(12) << shipping_cost_monero << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_sign(your_currency) << (neroshop::Converter::get_price(from, to) * shipping_cost) << " " << neroshop::string::upper(your_currency) << ")" <<  std::endl;
    std::cout << "Order total: " << std::fixed << std::setprecision(12) << total_monero << " xmr" << std::fixed << std::setprecision(2) << " (" << neroshop::Converter::get_currency_sign(your_currency) << (neroshop::Converter::get_price(from, to) * total) << " " << neroshop::string::upper(your_currency) << ")" <<  std::endl;
    //std::cout << "Estimated delivery date: " << delivery_date_est << std::endl;
    // set order status => pending
    set_status(OrderStatus::New); // if everything went well then order will be set to pending
    database->execute_params("UPDATE orders SET status = $1 WHERE uuid = $2", { get_status_as_string(), order_id });
    //-------------------------------------------------------
    database->execute("COMMIT;"); // end transaction
}
////////////////////
void neroshop::Order::cancel_order()
{
    // cannot cancel order if it has been at least 12 hours or more
    // sellers can request that a buyer cancels an order
    // only a buyer can cancel an order
    set_status( OrderStatus::Cancelled );
}
////////////////////
void neroshop::Order::change_order()
{}
////////////////////
////////////////////
void neroshop::Order::set_id(const std::string& id) {
    this->id = id;
}

void neroshop::Order::set_date(const std::string& date) {
    this->date = date;
}

void neroshop::Order::set_status(OrderStatus status) { 
    this->status = status;
}

void neroshop::Order::set_status_by_string(const std::string& status) {
    if(status == "New") set_status(OrderStatus::New);
    if(status == "Pending") set_status(OrderStatus::Pending);
    if(status == "Processing") set_status(OrderStatus::Processing);
    if(status == "Shipped") set_status(OrderStatus::Shipped);
    if(status == "Ready For Pickup") set_status(OrderStatus::ReadyForPickup);
    if(status == "Delivered") set_status(OrderStatus::Delivered);
    if(status == "Cancelled") set_status(OrderStatus::Cancelled);
    if(status == "Failed") set_status(OrderStatus::Failed);
    if(status == "Returned") set_status(OrderStatus::Returned);
    if(status == "Disputed") set_status(OrderStatus::Disputed);
    if(status == "Declined") set_status(OrderStatus::Declined);
    //if(status == "") set_status(OrderStatus::);
}

void neroshop::Order::set_customer_id(const std::string& customer_id) {
    this->customer_id = customer_id;
}

void neroshop::Order::set_subtotal(double subtotal) {
    this->subtotal = subtotal;
}

void neroshop::Order::set_discount(double discount) {
    this->discount = discount;
}

void neroshop::Order::set_shipping_cost(double shipping_cost) {
    this->shipping_cost = shipping_cost;
}

void neroshop::Order::set_total(double total) {
    this->total = total;
}

void neroshop::Order::set_payment_option(PaymentOption payment_option) {
    this->payment_option = payment_option;
}

void neroshop::Order::set_payment_option_by_string(const std::string& payment_option) {
    if(payment_option == "Escrow") set_payment_option(PaymentOption::Escrow);
    if(payment_option == "Multisig") set_payment_option(PaymentOption::Multisig);
    if(payment_option == "Finalize") set_payment_option(PaymentOption::Finalize);
}

void neroshop::Order::set_payment_coin(PaymentCoin payment_coin) {
    this->payment_coin = payment_coin;
}

void neroshop::Order::set_payment_coin_by_string(const std::string& payment_coin) {
    if(payment_coin == "None") set_payment_coin(PaymentCoin::None);
    if(payment_coin == "Monero") set_payment_coin(PaymentCoin::Monero);
    //if(payment_coin == "") set_payment_coin(PaymentCoin::);
}

void neroshop::Order::set_delivery_option(DeliveryOption delivery_option) {
    this->delivery_option = delivery_option;
}

void neroshop::Order::set_delivery_option_by_string(const std::string& delivery_option) {
    if(delivery_option == "Delivery") set_delivery_option(DeliveryOption::Delivery);
    if(delivery_option == "Pickup") set_delivery_option(DeliveryOption::Pickup);
}

void neroshop::Order::set_notes(const std::string& notes) {
    this->notes = notes;
}

void neroshop::Order::set_items(const std::vector<std::tuple<std::string, int, std::string>>& items) {
    this->items = items;
}
////////////////////
////////////////////
std::string neroshop::Order::get_id() const {
    return id;
}

neroshop::OrderStatus neroshop::Order::get_status() const {
    return status;
}

std::string neroshop::Order::get_status_as_string() const {
    switch(status) {
        case OrderStatus::New: return "New"; break;
        case OrderStatus::Pending: return "Pending"; break;
        case OrderStatus::Processing: return "Processing"; break;
        case OrderStatus::Shipped: return "Shipped"; break;
        case OrderStatus::ReadyForPickup: return "Ready For Pickup"; break; //case OrderStatus::Ready: return "Ready For Pickup"; break;
        case OrderStatus::Delivered: return "Delivered"; break; //case OrderStatus::Done: return "Delivered"; break;
        case OrderStatus::Cancelled: return "Cancelled"; break;
        case OrderStatus::Failed: return "Failed"; break;
        case OrderStatus::Returned: return "Returned"; break;
        case OrderStatus::Disputed: return "Disputed"; break;
        case OrderStatus::Declined: return "Declined"; break;
        //case OrderStatus::: return ""; break;
        default: return "New";
    }
    //return "New";
}

std::string neroshop::Order::get_date() const {
    return date;
}

std::string neroshop::Order::get_customer_id() const {
    return customer_id;
}

double neroshop::Order::get_subtotal() const {
    return subtotal;
}

double neroshop::Order::get_discount() const {
    return discount;
}

double neroshop::Order::get_shipping_cost() const {
    return shipping_cost;
}

double neroshop::Order::get_total() const {
    return total;
}

neroshop::PaymentOption neroshop::Order::get_payment_option() const {
    return payment_option;
}

std::string neroshop::Order::get_payment_option_as_string() const {
    switch(payment_option) {
        case PaymentOption::Escrow: return "Escrow"; break;
        case PaymentOption::Multisig: return "Multisig"; break;
        case PaymentOption::Finalize: return "Finalize"; break;
        default: return "Escrow";
    }
    //return "Escrow";
}

neroshop::PaymentCoin neroshop::Order::get_payment_coin() const {
    return payment_coin;
}

std::string neroshop::Order::get_payment_coin_as_string() const {
    switch(payment_coin) {
        case PaymentCoin::None: return "None"; break;
        case PaymentCoin::Monero: return "Monero"; break;
        //case PaymentCoin::: return ""; break;
        default: return "Monero";
    }
    //return "Monero";
}

neroshop::DeliveryOption neroshop::Order::get_delivery_option() const {
    return delivery_option;
}

std::string neroshop::Order::get_delivery_option_as_string() const {
    switch(delivery_option) {
        case DeliveryOption::Delivery: return "Delivery"; break;
        case DeliveryOption::Pickup: return "Pickup"; break;
        default: return "Delivery";
    }
    //return "Delivery";
}

std::string neroshop::Order::get_notes() const {
    return notes;
}

std::vector<std::tuple<std::string, int, std::string>> neroshop::Order::get_items() const {
    return items;
}
////////////////////
////////////////////
bool neroshop::Order::is_cancelled() const {
    return (status == OrderStatus::Cancelled);
}
////////////////////
////////////////////
