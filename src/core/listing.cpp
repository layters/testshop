#include "listing.hpp"

#include <utility> // std::exchange

neroshop::Listing::Listing() : quantity(0), price(0.00) {}

neroshop::Listing::Listing(const std::string& id, const std::string& product_id, const std::string& seller_id, unsigned int quantity,
        double price, const std::string& currency, const std::string& condition, const std::string& location, const std::string& date)
    : id(id), product_id(product_id), seller_id(seller_id), quantity(quantity),
      price(price), currency(currency), condition(condition), location(location), date(date)
{}

neroshop::Listing::Listing(const Listing& other)
    : id(other.id), product_id(other.product_id), seller_id(other.seller_id), quantity(other.quantity),
      price(other.price), currency(other.currency), condition(other.condition), location(other.location),
      date(other.date)
{}

neroshop::Listing::Listing(Listing&& other) noexcept
    : id(std::move(other.id)), product_id(std::move(other.product_id)), seller_id(std::move(other.seller_id)),
      quantity(std::exchange(other.quantity, 0)), price(std::exchange(other.price, 0.0)),
      currency(std::move(other.currency)), condition(std::move(other.condition)), location(std::move(other.location)),
      date(std::move(other.date))
{}

//-----------------------------------------------------------------------------

neroshop::Listing& neroshop::Listing::operator=(const neroshop::Listing& other)
{
    if (this != &other) {
        id = other.id;
        product_id = other.product_id;
        seller_id = other.seller_id;
        quantity = other.quantity;
        price = other.price;
        currency = other.currency;
        condition = other.condition;
        location = other.location;
        date = other.date;
    }
    return *this;
}

neroshop::Listing& neroshop::Listing::operator=(neroshop::Listing&& other) noexcept
{
    if (this != &other) {
        id = std::move(other.id);
        product_id = std::move(other.product_id);
        seller_id = std::move(other.seller_id);
        quantity = std::exchange(other.quantity, 0);
        price = std::exchange(other.price, 0.0);
        currency = std::move(other.currency);
        condition = std::move(other.condition);
        location = std::move(other.location);
        date = std::move(other.date);
    }
    return *this;
}

//-----------------------------------------------------------------------------

void neroshop::Listing::set_id(const std::string& id) { 
    this->id = id; 
}

void neroshop::Listing::set_product_id(const std::string& product_id) { 
    this->product_id = product_id; 
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

//-----------------------------------------------------------------------------

std::string neroshop::Listing::get_id() const {
    return id;
}

std::string neroshop::Listing::get_product_id() const {
    return product_id;
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
