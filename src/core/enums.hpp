#pragma once

#ifndef ENUMS_HPP_NEROSHOP
#define ENUMS_HPP_NEROSHOP

namespace neroshop {

// currency and price
enum class Currency {
    // Fiat
    USD = 0,
    EUR,
    JPY,
    GBP,
    CAD,
    CHF,
    AUD,
    CNY,
    SEK,
    NZD,
    MXN,
    NGN,
    GHS,
    RUB,
    PHP,
    INR,
    // Metals
    XAG,
    XAU,
    // Crypto
    XMR,
    BTC,
    ETH,
    LTC,
    WOW,
};

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

} // namespace neroshop
#endif
