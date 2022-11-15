#pragma once

#ifndef PRICE_SOURCE_FACTORY_HPP_NEROSHOP
#define PRICE_SOURCE_FACTORY_HPP_NEROSHOP

#include <memory>

#include "price_source.hpp"

namespace PriceSourceFactory {

enum class Source {
    CoinMarketCap = 0,
    CoinGecko,
    CryptoWatch,
    CoinTelegraph,
    CryptoRank,
    CoinCodex,
};

std::unique_ptr<PriceSource> makePriceSouce(Source source);

}; // namespace PriceSourceFactory

#endif // PRICESOURCEFACTORY_H
