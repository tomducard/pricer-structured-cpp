#pragma once

#include <string>
#include <unordered_map>

/**
 * @brief Container for market data.
 *
 * Holds risk-free rates and market quotes (spot prices, volatilities)
 * for various underlying assets.
 */
class MarketData {
public:
    struct Quote {
        double spot;
        double sigma;
    };

    void setRiskFreeRate(double r);
    double riskFreeRate() const;

    /**
     * @brief Stores a quote for a specific underlying.
     */
    void setQuote(const std::string& underlying, const Quote& quote);

    /**
     * @brief Retrieves the quote for a specific underlying.
     * @throws std::runtime_error if the underlying is not found.
     */
    const Quote& getQuote(const std::string& underlying) const;

private:
    double riskFreeRate_{0.0};
    std::unordered_map<std::string, Quote> quotes_;
};