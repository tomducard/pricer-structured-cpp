// Core interface every structured product must implement (payoff, timing, curve).
#pragma once

#include <string>
#include <utility>
#include <vector>

class StructuredProduct {
public:
    virtual ~StructuredProduct() = default;

    virtual double payoff(const std::vector<double>& path) const = 0;
    virtual std::pair<double, double> payoffAndPayTime(
        const std::vector<double>& path) const = 0;
    virtual double terminalRedemption(double spotT) const = 0;
    virtual const std::vector<double>& observationTimes() const = 0;
    virtual const std::string& underlying() const = 0;
};
