#ifndef COINCODEX_PRICE_SOURCE_HPP_NEROSHOP
#define COINCODEX_PRICE_SOURCE_HPP_NEROSHOP

#include "price_source.hpp"

class CoinCodexPriceSource : public PriceSource
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINCODEX_PRICE_SOURCE_HPP_NEROSHOP
