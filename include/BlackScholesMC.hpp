#pragma once

#include "PathModel.hpp"

class BlackScholesMC : public PathModelBase {
public:
    // On passe la volatilité au constructeur
    explicit BlackScholesMC(double sigma);

    std::vector<double> simulatePath(double spot0,
                                     const std::vector<double>& times,
                                     const MarketData& data,
                                     std::mt19937& rng) const override;
private:
    double sigma_;
};