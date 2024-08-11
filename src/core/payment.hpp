#pragma once

#include <iostream>
#include <string>

namespace neroshop {

// payment status
enum class PaymentStatus {
    Pending, // Payment has been initiated but has not been received yet
    Confirmed, // Payment has been confirmed and accepted by the blockchain network
    Completed, // Payment has been successfully completed and the transaction is final
    Partial, // Payment has been partially completed or only a portion of the requested amount has been received
    Refunded, // Payment has been refunded
    Expired, // Payment window has expired and the transaction cannot be completed
    Released, // Payment has been released to the seller after the transaction is complete
    Held, // Payment is being held in escrow until the transaction is completed or resolved
};

// payment methods - may not be necessary since only crypto will be used
enum class PaymentMethod {
    Crypto = 0,
    Cash, 
    Card, // Can be either credit, debit, or pre-paid
    DigitalApp, // Can include cashapp, paypal, etc.
    PreciousMetal,
    Goldback,
};

// payment options
enum class PaymentOption {
    Escrow = 0, // 2 of 3
    Multisig, // 2 of 2
    Finalize, // Direct payment (all non-crypto payment methods are finalize by default)
};

// payment coins (cryptocurrencies used for payments)
enum class PaymentCoin { 
    None = -1, // Intended for non-crypto payment methods
    Monero,
    Wownero,
};

//-----------------------------------------------------------------------------

inline std::string get_payment_method_as_string(PaymentMethod payment_method) {
    switch(payment_method) {
        case PaymentMethod::Crypto: return "Crypto";
        case PaymentMethod::Cash: return "Cash";
        case PaymentMethod::Card: return "Card";
        case PaymentMethod::DigitalApp: return "DigitalApp";
        case PaymentMethod::PreciousMetal: return "PreciousMetal";
        case PaymentMethod::Goldback: return "Goldback";
        default: return "";
    }
}

inline std::string get_payment_coin_as_string(PaymentCoin payment_coin) {
    switch(payment_coin) {
        case PaymentCoin::None: return "None";
        case PaymentCoin::Monero: return "XMR";
        case PaymentCoin::Wownero: return "WOW";
        default: return "";
    }
}

inline std::string get_payment_option_as_string(PaymentOption payment_option) {
    switch(payment_option) {
        case PaymentOption::Escrow: return "Escrow";
        case PaymentOption::Multisig: return "Multisig";
        case PaymentOption::Finalize: return "Finalize";
        default: return "";
    }
}

}
