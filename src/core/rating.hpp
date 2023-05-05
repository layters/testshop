#pragma once

namespace neroshop {

struct Rating {
    std::string rater_id;
    std::string comments;
    std::string signature;
};

struct ProductRating : public Rating {
    std::string product_id;
    unsigned int stars;
};

struct SellerRating : public Rating {
    std::string seller_id;
    unsigned int score;
};

}
