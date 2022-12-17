#ifndef CRYPTORANK_API_HPP_NEROSHOP
#define CRYPTORANK_API_HPP_NEROSHOP

#include "price_api.hpp"

class CryptoRankApi : public PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // CRYPTORANK_PRICE_SOURCE_HPP_NEROSHOP
