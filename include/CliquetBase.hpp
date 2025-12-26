#pragma once

#include "StructuredProduct.hpp"

#include <string>
#include <utility>
#include <vector>

class CliquetBase : public StructuredProduct {
public:
    CliquetBase(std::string underlying,
                std::vector<double> observationTimes,
                double spot0,
                double notional);

    double payoff(const std::vector<double>& path) const override;
    std::pair<double, double> payoffAndPayTime(
        const std::vector<double>& path) const override;
    double terminalRedemption(double spotT) const override;
    const std::vector<double>& observationTimes() const override;
    const std::string& underlying() const override;

protected:
    const std::vector<double>& times() const { return observationTimes_; }
    double spot0() const { return spot0_; }
    double notional() const { return notional_; }

private:
    virtual double payoffImpl(const std::vector<double>& path) const = 0;
    std::vector<double> buildReferencePath(double terminalSpot) const;

    std::string underlying_;
    std::vector<double> observationTimes_;
    double spot0_{};
    double notional_{};
};
