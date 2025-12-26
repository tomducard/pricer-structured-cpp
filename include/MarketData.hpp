#pragma once

#include <string>
#include <unordered_map>

class MarketData {
public:
    struct Quote {
        double spot;
        double sigma;
    };

    void setRiskFreeRate(double r);
    double riskFreeRate() const;

    void setQuote(const std::string& underlying, const Quote& q);
    Quote getQuote(const std::string& underlying) const;

private:
    double riskFreeRate_{0.0};
    std::unordered_map<std::string, Quote> quotes_;
};