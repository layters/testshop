#pragma once

#ifndef PRICE_API_HPP_NEROSHOP
#define PRICE_API_HPP_NEROSHOP

#include "currency_enum.hpp" // neroshop::Currency::

#include <optional>

namespace neroshop {

enum class PriceSource {
    // Crypto Price Aggregators
    CoinMarketCap = 0,
    CoinGecko,
    CryptoWatch,
    CoinTelegraph,
    CryptoRank,
    CoinCodex,
    Fawazahmed0,
    // Exchanges
    Kraken,
};

class PriceApi
{
public:
    //PriceApi() = default;
    virtual ~PriceApi() = default;

    virtual std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const = 0;
};

}
#endif // PRICE_API_HPP_NEROSHOP
