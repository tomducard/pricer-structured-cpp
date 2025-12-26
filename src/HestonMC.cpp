#include "HestonMC.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

HestonMC::HestonMC(double v0, double kappa, double theta, double xi, double rho)
    : v0_(v0), kappa_(kappa), theta_(theta), xi_(xi), rho_(rho) {}

std::vector<double> HestonMC::simulatePath(
    double spot0,
    const std::vector<double>& times,
    const MarketData& data,
    std::mt19937& rng) const {
    // Simple Euler scheme with full-truncation variance, correlated normals.
    std::vector<double> path(times.size());
    if (times.empty()) {
        return path;
    }

    std::normal_distribution<double> dist(0.0, 1.0);
    const double r = data.riskFreeRate();
    double spot = spot0;
    double var = std::max(v0_, 0.0);
    double prevTime = 0.0;

    for (std::size_t i = 0; i < times.size(); ++i) {
        const double dt = std::max(times[i] - prevTime, 0.0);
        if (dt > 0.0) {
            const double z1 = dist(rng);
            const double z2 = dist(rng);
            const double zVar =
                rho_ * z1 + std::sqrt(std::max(1.0 - rho_ * rho_, 0.0)) * z2;

            const double sqrtVar = std::sqrt(std::max(var, 0.0));
            const double varianceDrift = kappa_ * (theta_ - var) * dt;
            const double varianceDiff = xi_ * sqrtVar * std::sqrt(dt) * zVar;
            var = std::max(var + varianceDrift + varianceDiff, 0.0);
            const double spotDiffusionVar = std::max(var, 0.0);

            const double drift = (r - 0.5 * spotDiffusionVar) * dt;
            const double diffusion =
                std::sqrt(spotDiffusionVar) * std::sqrt(dt) * z1;
            spot *= std::exp(drift + diffusion);
        }

        path[i] = spot;
        prevTime = times[i];
    }

    return path;
}
