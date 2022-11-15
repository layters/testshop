#ifndef COINMARKETCAP_PRICE_SOURCE_HPP_NEROSHOP
#define COINMARKETCAP_PRICE_SOURCE_HPP_NEROSHOP

#include "price_source.hpp"

class CoinMarketCapPriceSource : public PriceSource
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINMARKETCAP_PRICE_SOURCE_HPP_NEROSHOP
