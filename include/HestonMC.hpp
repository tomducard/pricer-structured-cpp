#pragma once

#include "PathModel.hpp"

class HestonMC : public PathModelBase {
public:
    HestonMC(double v0, double kappa, double theta, double xi, double rho);

    std::vector<double> simulatePath(double spot0,
                                     const std::vector<double>& times,
                                     const MarketData& data,
                                     std::mt19937& rng) const override;

private:
    double v0_{};
    double kappa_{};
    double theta_{};
    double xi_{};
    double rho_{};
};

