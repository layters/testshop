#pragma once

#ifndef WOWNERO_HPP_NEROSHOP
#define WOWNERO_HPP_NEROSHOP

#include "../wallet.hpp"

namespace neroshop {

class WowneroWallet : public Wallet/*, public wownero_wallet_listener*/ {
public:
    WowneroWallet();
    ~WowneroWallet();
};

}
#endif // WOWNERO_HPP_NEROSHOP
