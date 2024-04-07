#ifndef FAWAZAHMED0_CURRENCY_API_HPP_NEROSHOP
#define FAWAZAHMED0_CURRENCY_API_HPP_NEROSHOP

#include "../price_api.hpp"

class Fawazahmed0CurrencyApi : public neroshop::PriceApi
{
public:
    std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const override;
};

#endif // FAWAZAHMED0_CURRENCY_API_HPP_NEROSHOP
