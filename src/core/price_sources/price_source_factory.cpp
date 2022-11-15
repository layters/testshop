#include "price_source_factory.hpp"

#include "coincodex.hpp"
#include "coingecko.hpp"
#include "coinmarketcap.hpp"
#include "cointelegraph.hpp"
#include "cryptorank.hpp"
#include "cryptowatch.hpp"

std::unique_ptr<PriceSource> PriceSourceFactory::makePriceSouce(Source source)
{
    switch (source) {
    case Source::CoinMarketCap:
        return std::make_unique<CoinMarketCapPriceSource>();
    case Source::CoinGecko:
        return std::make_unique<CoinGeckoPriceSource>();
    case Source::CryptoWatch:
        return std::make_unique<CryptoWatchPriceSource>();
    case Source::CoinTelegraph:
        return std::make_unique<CoinTelegraphPriceSource>();
    case Source::CryptoRank:
        return std::make_unique<CryptoRankPriceSource>();
    case Source::CoinCodex:
        return std::make_unique<CoinCodexPriceSource>();
    }
    return nullptr;
}
