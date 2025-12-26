#include "BlackScholesMC.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

BlackScholesMC::BlackScholesMC(double sigma) : sigma_(sigma) {}

std::vector<double> BlackScholesMC::simulatePath(
    double spot0,
    const std::vector<double>& times,
    const MarketData& data,
    std::mt19937& rng) const {

    std::vector<double> path(times.size());
    const double r = data.riskFreeRate();
    // On utilise le sigma stocké
    const double sigma = std::max(sigma_, 1e-8);
    const double sigmaSqHalf = 0.5 * sigma * sigma;

    std::normal_distribution<double> dist(0.0, 1.0);
    double spot = spot0;
    double prevTime = 0.0;

    for (std::size_t i = 0; i < times.size(); ++i) {
        const double dt = std::max(times[i] - prevTime, 0.0);
        if (dt > 0.0) {
            const double drift = (r - sigmaSqHalf) * dt;
            const double diffusion = sigma * std::sqrt(dt) * dist(rng);
            spot *= std::exp(drift + diffusion);
        }
        path[i] = spot;
        prevTime = times[i];
    }

    return path;
}