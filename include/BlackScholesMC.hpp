#pragma once

#include "PathModel.hpp"

class BlackScholesMC : public PathModelBase {
public:
    std::vector<double> simulatePath(double spot0,
                                     const std::vector<double>& times,
                                     const MarketData& data,
                                     std::mt19937& rng) const override;
};

