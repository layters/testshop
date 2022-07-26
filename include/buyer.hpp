//#pragma once
#ifndef BUYER_HPP_NEROSHOP
#define BUYER_HPP_NEROSHOP

#include <iostream>

#include "user.hpp" // includes "order.hpp"

namespace neroshop {
class Buyer : public User { // customers will not be required to register, unless they would like to save their order history
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
// https://calculator.academy/average-rating-calculator-star-rating/#f1p1|f2p0
// https://www.ebay.com/help/buying/resolving-issues-sellers/seller-ratings?id=4023&mkevt=1&mkcid=1&mkrid=711-53200-19255-0&campid=5336728181&customid=&toolid=10001

