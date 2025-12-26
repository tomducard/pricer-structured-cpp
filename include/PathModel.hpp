// Abstract interface for Monte Carlo path generators (BS, Heston, ...).
#pragma once

#include "MarketData.hpp"

#include <random>
#include <vector>

class PathModelBase {
public:
    virtual ~PathModelBase() = default;

    virtual std::vector<double> simulatePath(
        double spot0,
        const std::vector<double>& times,
        const MarketData& data,
        std::mt19937& rng) const = 0;
};
