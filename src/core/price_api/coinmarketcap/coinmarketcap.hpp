#ifndef COINMARKETCAP_API_HPP_NEROSHOP
#define COINMARKETCAP_API_HPP_NEROSHOP

#include "../price_api.hpp"

class CoinMarketCapApi : public neroshop::PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINMARKETCAP_PRICE_SOURCE_HPP_NEROSHOP
