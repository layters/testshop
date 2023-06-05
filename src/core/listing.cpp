#include "listing.hpp"

#include <utility> // std::exchange

#include "product.hpp"

neroshop::Listing::Listing() : quantity(0), price(0.00) {}

neroshop::Listing::Listing(const std::string& id, const Product& product, const std::string& seller_id, unsigned int quantity,
        double price, const std::string& currency, const std::string& condition, const std::string& location, const std::string& date)
    : id(id), product(std::make_unique<Product>(product)), seller_id(seller_id), quantity(quantity),
      price(price), currency(currency), condition(condition), location(location), date(date)
{}

neroshop::Listing::Listing(const Listing& other)
    : id(other.id), product(std::make_unique<Product>(*other.product)), seller_id(other.seller_id), quantity(other.quantity),
      price(other.price), currency(other.currency), condition(other.condition), location(other.location),
      date(other.date), signature(other.signature)
{}

neroshop::Listing::Listing(Listing&& other) noexcept
    : id(std::move(other.id)), product(std::move(other.product)), seller_id(std::move(other.seller_id)),
      quantity(std::exchange(other.quantity, 0)), price(std::exchange(other.price, 0.0)),
      currency(std::move(other.currency)), condition(std::move(other.condition)), location(std::move(other.location)),
      date(std::move(other.date)), signature(std::move(other.signature))
{}

//-----------------------------------------------------------------------------

neroshop::Listing& neroshop::Listing::operator=(const neroshop::Listing& other)
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
    }
    return *this;
}

neroshop::Listing& neroshop::Listing::operator=(neroshop::Listing&& other) noexcept
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
    }
    return *this;
}

//-----------------------------------------------------------------------------

void neroshop::Listing::print_listing()
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

void neroshop::Listing::set_id(const std::string& id) { 
    this->id = id; 
}

void neroshop::Listing::set_product_id(const std::string& product_id) { 
    this->product->set_id(product_id);
}

void neroshop::Listing::set_seller_id(const std::string& seller_id) { 
    this->seller_id = seller_id; 
}

void neroshop::Listing::set_quantity(unsigned int quantity) { 
    this->quantity = quantity; 
}

void neroshop::Listing::set_price(double price) { 
    this->price = price; 
}

void neroshop::Listing::set_currency(const std::string& currency) { 
    this->currency = currency; 
}

void neroshop::Listing::set_condition(const std::string& condition) { 
    this->condition = condition; 
}

void neroshop::Listing::set_location(const std::string& location) { 
    this->location = location; 
}

void neroshop::Listing::set_date(const std::string& date) { 
    this->date = date; 
}

void neroshop::Listing::set_product(const Product& product) {
    this->product = std::make_unique<Product>(product);
}

//-----------------------------------------------------------------------------

std::string neroshop::Listing::get_id() const {
    return id;
}

std::string neroshop::Listing::get_product_id() const {
    return product->get_id();
}

std::string neroshop::Listing::get_seller_id() const {
    return seller_id;
}

int neroshop::Listing::get_quantity() const {
    return quantity;
}

double neroshop::Listing::get_price() const {
    return price;
}

std::string neroshop::Listing::get_currency() const {
    return currency;
}

std::string neroshop::Listing::get_condition() const {
    return condition;
}

std::string neroshop::Listing::get_location() const {
    return location;
}

std::string neroshop::Listing::get_date() const {
    return date;
}

neroshop::Product * neroshop::Listing::get_product() const {
    return product.get();
}
