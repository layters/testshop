#ifndef COINTELEGRAPH_PRICE_SOURCE_HPP_NEROSHOP
#define COINTELEGRAPH_PRICE_SOURCE_HPP_NEROSHOP

#include "price_source.hpp"

class CoinTelegraphPriceSource : public PriceSource
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINTELEGRAPH_PRICE_SOURCE_HPP_NEROSHOP
