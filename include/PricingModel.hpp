#pragma once

#include "MarketData.hpp"
#include "StructuredProduct.hpp"

class PricingModel {
public:
    virtual ~PricingModel() = default;

    virtual double price(const StructuredProduct& product,
                         const MarketData& data) const = 0;
};
