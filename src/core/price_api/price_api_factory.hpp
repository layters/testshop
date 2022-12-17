#pragma once

#ifndef PRICE_API_FACTORY_HPP_NEROSHOP
#define PRICE_API_FACTORY_HPP_NEROSHOP

#include <memory>

#include "price_api.hpp"

namespace PriceApiFactory {

enum class Source {
    CoinMarketCap = 0,
    CoinGecko,
    CryptoWatch,
    CoinTelegraph,
    CryptoRank,
    CoinCodex,
};

std::unique_ptr<PriceApi> makePriceSouce(Source source);

}; // namespace PriceApiFactory

#endif // PRICESOURCEFACTORY_H
