#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <set>

#include "payment.hpp"
#include "delivery.hpp"

namespace neroshop {

class Product;

class Listing {
public:
    Listing();
    Listing(const std::string& id, const Product& product, const std::string& seller_id, unsigned int quantity,
            double price, const std::string& currency, const std::string& condition, const std::string& location, const std::string& date, const std::string& signature,
            unsigned int quantity_per_order);
    Listing(const Listing& other);// copy constructor
    Listing(Listing&& other) noexcept; // move constructor
    
    Listing& operator=(const Listing&); // copy assignment operator
    Listing& operator=(Listing&&) noexcept; // move assignment operator
    
    void add_payment_option(PaymentOption payment_option);
    void add_delivery_option(DeliveryOption delivery_option);
    void print_listing();
    
    void set_id(const std::string& id);
    void set_product_id(const std::string& product_id);
    void set_seller_id(const std::string& seller_id);
    void set_quantity(unsigned int quantity);
    void set_price(double price);
    void set_currency(const std::string& currency);
    void set_condition(const std::string& condition);
    void set_location(const std::string& location);
    void set_date(const std::string& date);
    void set_product(const Product& product);
    void set_signature(const std::string& signature);
    void set_quantity_per_order(unsigned int quantity_per_order);
    //void set_payment_method(PaymentMethod payment_method);
    void set_payment_coin(PaymentCoin payment_coin);
    void set_payment_options(const std::set<PaymentOption>& payment_options);
    void set_delivery_options(const std::set<DeliveryOption>& delivery_options);
    
    std::string get_id() const;
    std::string get_product_id() const;
    std::string get_seller_id() const;
    int get_quantity() const;
    double get_price() const;
    std::string get_currency() const;
    std::string get_condition() const;
    std::string get_location() const;
    std::string get_date() const;
    Product * get_product() const;
    std::string get_signature() const;
    int get_quantity_per_order() const;
    PaymentMethod get_payment_method() const;
    PaymentCoin get_payment_coin() const;
    std::set<PaymentOption> get_payment_options() const;
    std::set<DeliveryOption> get_delivery_options() const;
private:
    std::string id;
    std::string seller_id;
    unsigned int quantity;
    double price; // unit price or price per unit
    std::string currency;
    std::string condition; // TODO: make this an enum or nah?
    std::string location;
    std::string date; // date the listing was `created_at`
    std::string signature;
    std::unique_ptr<Product> product;
    unsigned int quantity_per_order;
    PaymentMethod payment_method; // default: PaymentMethod::Crypto (can never be changed)
    PaymentCoin payment_coin; // default: PaymentCoin::Monero
    std::set<PaymentOption> payment_options; // default: [ PaymentOption::Escrow ]
    std::set<DeliveryOption> delivery_options; // default: [ DeliveryOption::Delivery ]
};

}      

