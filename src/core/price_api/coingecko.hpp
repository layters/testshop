#ifndef COINGECKO_API_HPP_NEROSHOP
#define COINGECKO_API_HPP_NEROSHOP

#include "price_api.hpp"

class CoinGeckoApi : public PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINGECKO_PRICE_SOURCE_HPP_NEROSHOP
