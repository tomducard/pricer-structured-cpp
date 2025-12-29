#pragma once

#include "PathModel.hpp"

class BlackScholesMC : public PathModelBase {
public:
    // Pass volatility to the constructor
    explicit BlackScholesMC(double sigma);

    std::vector<double> simulatePath(double spot0,
                                     const std::vector<double>& times,
                                     const MarketData& data,
                                     std::mt19937& rng) const override;
private:
    double sigma_;
};