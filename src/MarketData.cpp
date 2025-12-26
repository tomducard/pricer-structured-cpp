#include "MarketData.hpp"
#include <stdexcept>

void MarketData::setRiskFreeRate(double r) {
    riskFreeRate_ = r;
}

double MarketData::riskFreeRate() const {
    return riskFreeRate_;
}

void MarketData::setQuote(const std::string& underlying, const Quote& q) {
    quotes_[underlying] = q;
}

MarketData::Quote MarketData::getQuote(const std::string& underlying) const {
    auto it = quotes_.find(underlying);
    if (it == quotes_.end()) {
        throw std::runtime_error("MarketData: quote not found for " + underlying);
    }
    return it->second;
}