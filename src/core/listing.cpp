#include "listing.hpp"

#include <utility> // std::exchange

#include "product.hpp"

namespace neroshop {

Listing::Listing() : quantity(0), price(0.00), quantity_per_order(0) {}

Listing::Listing(const std::string& id, const Product& product, const std::string& seller_id, unsigned int quantity,
        double price, const std::string& currency, const std::string& condition, const std::string& location, const std::string& date, const std::string& signature,
        unsigned int quantity_per_order, PaymentMethod payment_method, const std::set<PaymentCoin>& payment_coins, 
        const std::set<PaymentOption>& payment_options, const std::set<DeliveryOption>& delivery_options,
        const std::set<ShippingOption>& shipping_options)
    : id(id), product(std::make_unique<Product>(product)), seller_id(seller_id), quantity(quantity),
      price(price), currency(currency), condition(condition), location(location), date(date), signature(signature),
      quantity_per_order(quantity_per_order), payment_method(payment_method), payment_coins(payment_coins),
      payment_options(payment_options), delivery_options(delivery_options),
      shipping_options(shipping_options)
{}

Listing::Listing(const Listing& other)
    : id(other.id), product(std::make_unique<Product>(*other.product)), seller_id(other.seller_id), quantity(other.quantity),
      price(other.price), currency(other.currency), condition(other.condition), location(other.location),
      date(other.date), signature(other.signature),
      quantity_per_order(other.quantity_per_order), payment_method(other.payment_method), payment_coins(other.payment_coins),
      payment_options(other.payment_options), delivery_options(other.delivery_options),
      shipping_options(other.shipping_options)
{}

Listing::Listing(Listing&& other) noexcept
    : id(std::move(other.id)), product(std::move(other.product)), seller_id(std::move(other.seller_id)),
      quantity(std::exchange(other.quantity, 0)), price(std::exchange(other.price, 0.0)),
      currency(std::move(other.currency)), condition(std::move(other.condition)), location(std::move(other.location)),
      date(std::move(other.date)), signature(std::move(other.signature)),
      quantity_per_order(std::exchange(other.quantity_per_order, 0)), payment_method(std::move(other.payment_method)), payment_coins(std::move(other.payment_coins)),
      payment_options(std::move(other.payment_options)), delivery_options(std::move(other.delivery_options)),
      shipping_options(std::move(other.shipping_options))
{}

//-----------------------------------------------------------------------------

neroshop::Listing& Listing::operator=(const neroshop::Listing& other)
{
    if (this != &other) {
        id = other.id;
        product = std::make_unique<Product>(*other.product);
        seller_id = other.seller_id;
        quantity = other.quantity;
        price = other.price;
        currency = other.currency;
        condition = other.condition;
        location = other.location;
        date = other.date;
        signature = other.signature;
        quantity_per_order = other.quantity_per_order;
        payment_method = other.payment_method;
        payment_coins = other.payment_coins;
        payment_options = other.payment_options;
        delivery_options = other.delivery_options;
        shipping_options = other.shipping_options;
    }
    return *this;
}

neroshop::Listing& Listing::operator=(neroshop::Listing&& other) noexcept
{
    if (this != &other) {
        id = std::move(other.id);
        product = std::move(other.product);
        seller_id = std::move(other.seller_id);
        quantity = std::exchange(other.quantity, 0);
        price = std::exchange(other.price, 0.0);
        currency = std::move(other.currency);
        condition = std::move(other.condition);
        location = std::move(other.location);
        date = std::move(other.date);
        signature = std::move(other.signature);
        quantity_per_order = std::exchange(other.quantity_per_order, 0);
        payment_method = std::move(other.payment_method);
        payment_coins = std::move(other.payment_coins);
        payment_options = std::move(other.payment_options);
        delivery_options = std::move(other.delivery_options);
        shipping_options = std::move(other.shipping_options);
    }
    return *this;
}

//-----------------------------------------------------------------------------

void Listing::add_payment_coin(PaymentCoin payment_coin) {
    payment_coins.insert(payment_coin);
}

void Listing::add_payment_option(PaymentOption payment_option) {
    payment_options.insert(payment_option);
}

void Listing::add_delivery_option(DeliveryOption delivery_option) {
    delivery_options.insert(delivery_option);
}

void Listing::add_shipping_option(ShippingOption shipping_option) {
    shipping_options.insert(shipping_option);
}

void Listing::print_listing()
{
    std::cout << "Listing ID: " << id << std::endl;
    std::cout << "Seller ID: " << seller_id << std::endl;
    std::cout << "Quantity: " << quantity << std::endl;
    std::cout << "Price: " << price << " " << currency << std::endl;
    std::cout << "Condition: " << condition << std::endl;
    std::cout << "Location: " << location << std::endl;
    std::cout << "Date: " << date << std::endl;
    // Additional properties can be printed here

    if (product.get()) {
        std::cout << "Product Details:" << std::endl;
        std::cout << "-----------------" << std::endl;
        product->print_product();
    }
    else {
        std::cout << "No product associated with the listing." << std::endl;
    }
}

//-----------------------------------------------------------------------------

void Listing::set_id(const std::string& id) { 
    this->id = id; 
}

void Listing::set_product_id(const std::string& product_id) { 
    if (product == nullptr) {
        throw std::runtime_error("product is nullptr");
    }
    this->product->set_id(product_id);
}

void Listing::set_seller_id(const std::string& seller_id) { 
    this->seller_id = seller_id; 
}

void Listing::set_quantity(unsigned int quantity) { 
    this->quantity = quantity; 
}

void Listing::set_price(double price) { 
    this->price = price; 
}

void Listing::set_currency(const std::string& currency) { 
    this->currency = currency; 
}

void Listing::set_condition(const std::string& condition) { 
    this->condition = condition; 
}

void Listing::set_location(const std::string& location) { 
    this->location = location; 
}

void Listing::set_date(const std::string& date) { 
    this->date = date; 
}

void Listing::set_product(const Product& product) {
    this->product = std::make_unique<Product>(product);
}

void Listing::set_signature(const std::string& signature) {
    this->signature = signature;
}

void Listing::set_quantity_per_order(unsigned int quantity_per_order) {
    this->quantity_per_order = quantity_per_order;
}

void Listing::set_payment_method(PaymentMethod payment_method) {
    this->payment_method = payment_method;
}

void Listing::set_payment_coins(const std::set<PaymentCoin>& payment_coins) {
    this->payment_coins = payment_coins;
}

void Listing::set_payment_options(const std::set<PaymentOption>& payment_options) {
    this->payment_options = payment_options;
}

void Listing::set_delivery_options(const std::set<DeliveryOption>& delivery_options) {
    this->delivery_options = delivery_options;
}

void Listing::set_shipping_options(const std::set<ShippingOption>& shipping_options) {
    this->shipping_options = shipping_options;
}

void Listing::set_shipping_cost(ShippingOption shipping_option, double price) {
    shipping_costs[shipping_option] = price;
}

void Listing::set_shipping_costs(const std::map<ShippingOption, double>& shipping_costs) {
    this->shipping_costs = shipping_costs;
}

void Listing::set_custom_rate(PaymentCoin payment_coin, double rate) {
    custom_rates[payment_coin] = rate;
}

void Listing::set_custom_rates(const std::map<PaymentCoin, double>& custom_rates) {
    this->custom_rates = custom_rates;
}

//-----------------------------------------------------------------------------

std::string Listing::get_id() const {
    return id;
}

std::string Listing::get_product_id() const {
    if (product.get() == nullptr) {
        throw std::runtime_error("product is nullptr");
    }
    return product->get_id();
}

std::string Listing::get_seller_id() const {
    return seller_id;
}

int Listing::get_quantity() const {
    return quantity;
}

double Listing::get_price() const {
    return price;
}

std::string Listing::get_currency() const {
    return currency;
}

std::string Listing::get_condition() const {
    return condition;
}

std::string Listing::get_location() const {
    return location;
}

std::string Listing::get_date() const {
    return date;
}

neroshop::Product * Listing::get_product() const {
    return product.get();
}

std::string Listing::get_signature() const {
    return signature;
}

int Listing::get_quantity_per_order() const {
    return quantity_per_order;
}

PaymentMethod Listing::get_payment_method() const {
    return payment_method;
}

std::set<PaymentCoin> Listing::get_payment_coins() const {
    return payment_coins;
}

std::set<PaymentOption> Listing::get_payment_options() const {
    return payment_options;
}

std::set<DeliveryOption> Listing::get_delivery_options() const {
    return delivery_options;
}

std::set<ShippingOption> Listing::get_shipping_options() const {
    return shipping_options;
}

double Listing::get_shipping_cost(ShippingOption shipping_option) const {
    auto it = shipping_costs.find(shipping_option);
    if (it != shipping_costs.end()) {
        return it->second;
    }
    return 0.0;
}

std::map<ShippingOption, double> Listing::get_shipping_costs() const {
    return shipping_costs;
}

double Listing::get_custom_rate(PaymentCoin payment_coin) const {
    auto it = custom_rates.find(payment_coin);
    if (it != custom_rates.end()) {
        return it->second;
    }
    return 0.0;
}

std::map<PaymentCoin, double> Listing::get_custom_rates() const {
    return custom_rates;
}

}
