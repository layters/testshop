#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../../market/seller.hpp"
#include "../../market/product.hpp"
#include "../../market/listing.hpp"
#include "../../market/cart.hpp"
#include "../../market/order.hpp"
#include "../../market/rating.hpp"
#include "../../market/message.hpp"

namespace neroshop {

using Object = std::variant<User, Product, Listing, Cart, Order, ProductRating, SellerRating, Message>;

class Serializer {
private:
public:
    static std::pair<std::string, std::string/*std::vector<uint8_t>*/> serialize(const Object& object);
    static std::pair<std::string, std::string> serialize(const User& user);
    static std::shared_ptr<Object> deserialize(const std::pair<std::string, std::string/*std::vector<uint8_t>*/>& data);
    static std::shared_ptr<User> deserialize_user(const std::pair<std::string, std::string>& data);
};

}
