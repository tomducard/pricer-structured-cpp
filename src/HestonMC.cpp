#include "HestonMC.hpp"

#include <algorithm>
#include <cmath>
#include <random>

HestonMC::HestonMC(double v0, double kappa, double theta, double xi, double rho)
    : v0_(v0), kappa_(kappa), theta_(theta), xi_(xi), rho_(rho) {}

std::vector<double> HestonMC::simulatePath(double spot0,
                                           const std::vector<double>& times,
                                           const MarketData& data,
                                           std::mt19937& rng) const {
    std::vector<double> path(times.size());
    const double r = data.riskFreeRate();

    std::normal_distribution<double> dist(0.0, 1.0);

    double spot = spot0;
    double v = v0_; // Current variance state
    double prevTime = 0.0;

    // Use a finer time step for simulation accuracy (sub-stepping)
    // to avoid discretization errors with the stochastic volatility.
    const double dtStep = 0.01; // Max time step size

    for (std::size_t i = 0; i < times.size(); ++i) {
        double currentTime = prevTime;
        const double targetTime = times[i];

        while (currentTime < targetTime) {
            // Calculate actual time step for this iteration
            const double dt = std::min(dtStep, targetTime - currentTime);
            if (dt <= 1e-8) break;

            // Generate correlated Brownian motions
            const double z1 = dist(rng); // For spot
            const double z2 = dist(rng); // Uncorrelated
            // Correlated noise for variance:
            const double zv = rho_ * z1 + std::sqrt(1.0 - rho_ * rho_) * z2;

            // Update Variance (using Reflection or Truncation to keep v >= 0)
            // Here we use a simple full truncation scheme for stability:
            const double v_plus = std::max(v, 0.0);
            const double sqrt_v = std::sqrt(v_plus);

            // dv = kappa * (theta - v) * dt + xi * sqrt(v) * dW_v
            v += kappa_ * (theta_ - v_plus) * dt + xi_ * sqrt_v * std::sqrt(dt) * zv;

            // Update Spot
            // dS = S * r * dt + S * sqrt(v) * dW_s
            spot *= std::exp((r - 0.5 * v_plus) * dt + sqrt_v * std::sqrt(dt) * z1);

            currentTime += dt;
        }

        path[i] = spot;
        prevTime = targetTime;
    }

    return path;
}