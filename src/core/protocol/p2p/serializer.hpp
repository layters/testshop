#pragma once

#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "../../seller.hpp"
#include "../../product.hpp"
#include "../../listing.hpp"
#include "../../cart.hpp"
#include "../../order.hpp"
#include "../../rating.hpp"

namespace neroshop {

using Object = std::variant<User, Product, Listing, Cart, Order, ProductRating, SellerRating>;

class Serializer {
private:
public:
    static std::pair<std::string, std::string/*std::vector<uint8_t>*/> serialize(const Object& object);
    static std::pair<std::string, std::string> serialize(const User& user);
    static std::shared_ptr<Object> deserialize(const std::pair<std::string, std::string/*std::vector<uint8_t>*/>& data);//static const Object& deserialize(const std::string& json_object);
    static std::shared_ptr<User> deserialize_user(const std::pair<std::string, std::string>& data);
    
    template <typename... Args>
    static std::string to_json(Args&&... args);
};

}
