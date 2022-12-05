#include "price_api_factory.hpp"

#include "coincodex.hpp"
#include "coingecko.hpp"
#include "coinmarketcap.hpp"
#include "cointelegraph.hpp"
#include "cryptorank.hpp"
#include "cryptowatch.hpp"

std::unique_ptr<PriceApi> PriceApiFactory::makePriceSouce(Source source)
{
    switch (source) {
    case Source::CoinMarketCap:
        return std::make_unique<CoinMarketCapApi>();
    case Source::CoinGecko:
        return std::make_unique<CoinGeckoApi>();
    case Source::CryptoWatch:
        return std::make_unique<CryptoWatchApi>();
    case Source::CoinTelegraph:
        return std::make_unique<CoinTelegraphApi>();
    case Source::CryptoRank:
        return std::make_unique<CryptoRankApi>();
    case Source::CoinCodex:
        return std::make_unique<CoinCodexApi>();
    }
    return nullptr;
}
