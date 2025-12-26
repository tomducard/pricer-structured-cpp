#include "MarketData.hpp"

#include <stdexcept>
#include <utility>

void MarketData::setRiskFreeRate(double rate) {
    riskFreeRate_ = rate;
}

double MarketData::riskFreeRate() const {
    return riskFreeRate_;
}

void MarketData::setQuote(const std::string& name, Quote quote) {
    quotes_[name] = quote;
}

const MarketData::Quote& MarketData::getQuote(const std::string& name) const {
    const auto it = quotes_.find(name);
    if (it == quotes_.end()) {
        throw std::runtime_error("missing quote for underlying: " + name);
    }
    return it->second;
}

void MarketData::setVolProvider(std::shared_ptr<IVolProvider> provider) {
    volProvider_ = std::move(provider);
}

std::shared_ptr<IVolProvider> MarketData::volProvider() const {
    return volProvider_;
}
