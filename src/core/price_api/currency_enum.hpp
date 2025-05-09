#pragma once

#ifndef CURRENCY_ENUM_HPP_NEROSHOP
#define CURRENCY_ENUM_HPP_NEROSHOP

namespace neroshop {

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
    BRL,
    HKD,
    KRW,
    SGD,
    ZAR,
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

} // namespace neroshop
#endif
