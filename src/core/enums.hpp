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

// username and password verification
enum class PasswordResult {
    Password_Ok = 0,
    Password_NoUpperCaseLetter,
    Password_NoLowerCaseLetter,
    Password_NoDigit,
    Password_NoSpecialCharacter,
    Password_LengthTooShort, // or WrongLength
};

enum class UsernameResult {
    Username_Ok = 0,
    Username_LengthTooShort, // MinimumLengthReached
    Username_LengthTooLong, // MaximumLengthReached
    Username_NoSpacesAllowed,
    Username_NoSymbolsAllowedWithExceptions,
    Username_MustBeginWithLetter,
    Username_MustEndWithAlphaNumericCharacter,
    Username_TakenOrUnavailable,
    Username_ReservedForInternalUse,    
};

// wallet
enum class WalletResult {
    Wallet_Ok = 0, 
    Wallet_WrongPassword, 
    Wallet_PasswordsDoNotMatch, 
    Wallet_AlreadyExists,
    Wallet_OpenedByAnotherProgram, //Wallet_IOError,
};

} // namespace neroshop
#endif
