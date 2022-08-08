//#pragma once
#ifndef BUYER_HPP_NEROSHOP
#define BUYER_HPP_NEROSHOP

#include <iostream>

#include "user.hpp" // includes "order.hpp"

namespace neroshop {
class Buyer : public User { 
public:
    Buyer();
    Buyer(const std::string& name);
    ~Buyer();
    // callbacks
    static neroshop::User * on_login(const std::string& username);
private:
    //std::string get_contact_information() const;
    std::string get_shipping_address() const;
}; // save buyer's cart data, buyer can download order invoice
}
#endif

