#ifndef COINCODEX_API_HPP_NEROSHOP
#define COINCODEX_API_HPP_NEROSHOP

#include "price_api.hpp"

class CoinCodexApi : public PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINCODEX_PRICE_SOURCE_HPP_NEROSHOP
