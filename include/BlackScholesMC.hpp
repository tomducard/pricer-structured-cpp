#pragma once

#include "PathModel.hpp"

/**
 * @brief Black-Scholes Monte Carlo Path Generator.
 *
 * This class implements a path generator based on the Black-Scholes-Merton model.
 * It assumes the underlying asset follows a Geometric Brownian Motion (GBM)
 * with constant volatility and a constant risk-free rate.
 */
class BlackScholesMC : public PathModelBase {
public:
    /**
     * @brief Constructor.
     *
     * @param sigma The constant volatility of the underlying asset (e.g., 0.20 for 20%).
     */
    explicit BlackScholesMC(double sigma);

    /**
     * @brief Simulates a single price path using Geometric Brownian Motion.
     *
     * The evolution of the spot price is given by:
     * dS_t = S_t * r * dt + S_t * sigma * dW_t
     *
     * In the discretized simulation:
     * S_{t+dt} = S_t * exp((r - 0.5 * sigma^2) * dt + sigma * sqrt(dt) * Z)
     * where Z is a standard normal random variable.
     *
     * @param spot0 The initial spot price of the underlying.
     * @param times A vector of time points (in years) where the spot price is observed.
     * @param data Market data providing the risk-free rate (r).
     * @param rng The random number generator (Mersenne Twister) used to generate Z.
     * @return std::vector<double> The simulated spot prices at each requested time in 'times'.
     */
    std::vector<double> simulatePath(double spot0,
                                     const std::vector<double>& times,
                                     const MarketData& data,
                                     std::mt19937& rng) const override;

private:
    double sigma_; // stored constant volatility
};