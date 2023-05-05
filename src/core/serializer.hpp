#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "seller.hpp"
#include "product.hpp"
#include "listing.hpp"
#include "cart.hpp"
#include "order.hpp"
#include "rating.hpp"

namespace neroshop {

using Object = std::variant<Seller, Product, Listing, Cart, Order, ProductRating, SellerRating>;

class Serializer {
private:
public:
    static std::pair<std::string, std::string> serialize(const Object& object);
    static std::shared_ptr<Object> deserialize(const std::string& json_object);//static const Object& deserialize(const std::string& json_object);
    
    template <typename... Args>
    static std::string to_json(Args&&... args);
};

}
