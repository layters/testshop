#pragma once

#ifndef PRICE_SOURCE_HPP_NEROSHOP
#define PRICE_SOURCE_HPP_NEROSHOP

#include "../../core/currency_converter.hpp"

#include <optional>

class PriceSource
{
public:
    virtual ~PriceSource() = default;

    virtual std::optional<double> price(neroshop::Currency from, neroshop::Currency to) const = 0;
};

#endif // PRICESOURCE_H
