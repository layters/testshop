#ifndef CRYPTOWATCH_API_HPP_NEROSHOP
#define CRYPTOWATCH_API_HPP_NEROSHOP

#include "price_api.hpp"

class CryptoWatchApi : public PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // CRYPTOWATCH_PRICE_SOURCE_HPP_NEROSHOP
