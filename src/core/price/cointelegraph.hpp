#ifndef COINTELEGRAPH_API_HPP_NEROSHOP
#define COINTELEGRAPH_API_HPP_NEROSHOP

#include "price_api.hpp"

class CoinTelegraphApi : public PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // COINTELEGRAPH_PRICE_SOURCE_HPP_NEROSHOP
