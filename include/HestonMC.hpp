#pragma once

#include "PathModel.hpp"

/**
 * @brief Heston Monte Carlo Model implementation.
 *
 * This class simulates paths using the Heston stochastic volatility model.
 * It handles the time-discretization of both the spot price and the variance process.
 */
class HestonMC : public PathModelBase {
public:
    /**
     * @brief Constructor for the Heston Model.
     * @param v0 Initial variance.
     * @param kappa Mean reversion speed of the variance.
     * @param theta Long-term mean variance.
     * @param xi Volatility of volatility (vol-of-vol).
     * @param rho Correlation between spot and variance Brownian motions.
     */
    HestonMC(double v0, double kappa, double theta, double xi, double rho);

    /**
     * @brief Simulates a path using the Heston model.
     *
     * Uses an Euler-Maruyama discretization (or similar scheme) for the coupled SDEs.
     *
     * @param spot0 Initial spot price.
     * @param times Observation times required by the product.
     * @param data Market data (risk-free rate, etc.).
     * @param rng Random number generator.
     * @return std::vector<double> The simulated path of the underlying asset.
     */
    std::vector<double> simulatePath(double spot0,
                                     const std::vector<double>& times,
                                     const MarketData& data,
                                     std::mt19937& rng) const override;

private:
    double v0_;    // Initial variance
    double kappa_; // Mean reversion speed
    double theta_; // Long-term variance
    double xi_;    // Vol of vol
    double rho_;   // Correlation between spot and vol
};