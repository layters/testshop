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

// order status
enum class OrderStatus : unsigned int {
    Order_Incomplete, 
    Order_Created, 
    Order_Pending = Order_Created, 
    Order_Preparing, 
    Order_Shipped, 
    Order_ReadyForPickup, 
    Order_Ready = Order_ReadyForPickup, 
    Order_Delivered, 
    Order_Done = Order_Delivered, 
    Order_Cancelled, 
    Order_Failed, 
    Order_Returned,    
};

// payment status
enum class PaymentStatus {
    Payment_NotReceived, // red
    Payment_Confirmed, // yellow
    Payment_Received, // green
};

// payment options
enum class PaymentOption {
    Escrow, // 2 of 3
    Multisig, // 2 of 2
    Finalize, 
};

// payment methods - probably not necessary
enum class PaymentMethod {
    Cash, 
    Card, // can be either credit or debit
    Crypto, 
};

} // namespace neroshop
#endif
