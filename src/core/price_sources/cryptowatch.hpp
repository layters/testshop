#ifndef CRYPTOWATCH_PRICE_SOURCE_HPP_NEROSHOP
#define CRYPTOWATCH_PRICE_SOURCE_HPP_NEROSHOP

#include "price_source.hpp"

class CryptoWatchPriceSource : public PriceSource
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // CRYPTOWATCH_PRICE_SOURCE_HPP_NEROSHOP
