#ifndef COINGECKO_PRICE_SOURCE_HPP_NEROSHOP
#define COINGECKO_PRICE_SOURCE_HPP_NEROSHOP

#include "price_source.hpp"

class CoinGeckoPriceSource : public PriceSource
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINGECKO_PRICE_SOURCE_HPP_NEROSHOP
