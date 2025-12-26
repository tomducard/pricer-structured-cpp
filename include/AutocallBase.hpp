// Base class for single-underlying autocall variants: stores common data and
// provides shared payoff plumbing (payoff/discount timing, terminal redemption).
#pragma once

#include "StructuredProduct.hpp"

#include <string>
#include <utility>
#include <vector>

class AutocallBase : public StructuredProduct {
public:
    AutocallBase(std::string underlying,
                 std::vector<double> observationTimes,
                 double spot0,
                 double notional,
                 double couponRate,
                 double callBarrier,
                 double protectionBarrier);

    double payoff(const std::vector<double>& path) const override;
    const std::vector<double>& observationTimes() const override;
    const std::string& underlying() const override;
    double terminalRedemption(double spotT) const override;

protected:
    const std::vector<double>& times() const { return observationTimes_; }
    double spot0() const { return spot0_; }
    double notional() const { return notional_; }
    double couponRate() const { return couponRate_; }
    double callBarrier() const { return callBarrier_; }
    double protectionBarrier() const { return protectionBarrier_; }

protected:
    virtual std::pair<double, double> payoffAndPayTimeImpl(
        const std::vector<double>& path) const = 0;

private:
    std::pair<double, double> payoffAndPayTime(
        const std::vector<double>& path) const override;

    std::string underlying_;
    std::vector<double> observationTimes_;
    double spot0_{};
    double notional_{};
    double couponRate_{};
    double callBarrier_{};
    double protectionBarrier_{};
};
