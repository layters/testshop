#ifndef CRYPTORANK_PRICE_SOURCE_HPP_NEROSHOP
#define CRYPTORANK_PRICE_SOURCE_HPP_NEROSHOP

#include "price_source.hpp"

class CryptoRankPriceSource : public PriceSource
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // CRYPTORANK_PRICE_SOURCE_HPP_NEROSHOP
