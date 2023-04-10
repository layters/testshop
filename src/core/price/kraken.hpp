#ifndef KRAKEN_API_HPP_NEROSHOP
#define KRAKEN_API_HPP_NEROSHOP

#include "price_api.hpp"

class KrakenApi : public PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // KRAKEN_API_HPP_NEROSHOP
