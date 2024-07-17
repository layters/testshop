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

// payment options
enum class PaymentOption {
    Escrow = 0, // 2 of 3
    Multisig, // 2 of 2
    Finalize, // Direct payment (all non-crypto payment methods are finalize by default)
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

// payment coins (cryptocurrencies used for payments)
enum class PaymentCoin { 
    None = -1, // Intended for non-crypto payment methods
    Monero,
    Wownero,
};

}
