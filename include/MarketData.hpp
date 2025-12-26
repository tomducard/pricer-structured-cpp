// Simple container for risk-free rate, spot quotes, and a volatility provider.
#pragma once

#include "IVolProvider.hpp"

#include <memory>
#include <string>
#include <unordered_map>

class MarketData {
public:
    struct Quote {
        double spot{};
        double volatility{}; // annualized volatility
    };

    void setRiskFreeRate(double rate);
    double riskFreeRate() const;

    void setQuote(const std::string& name, Quote quote);
    const Quote& getQuote(const std::string& name) const;

    void setVolProvider(std::shared_ptr<IVolProvider> provider);
    std::shared_ptr<IVolProvider> volProvider() const;

private:
    double riskFreeRate_{0.0};
    std::unordered_map<std::string, Quote> quotes_;
    std::shared_ptr<IVolProvider> volProvider_;
};
