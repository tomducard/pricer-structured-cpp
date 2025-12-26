// Cliquet with capped positive coupons per period, summed at maturity.
#pragma once

#include "CliquetBase.hpp"

class CliquetCappedCoupons : public CliquetBase {
public:
    CliquetCappedCoupons(std::string underlying,
                         std::vector<double> observationTimes,
                         double spot0,
                         double notional,
                         double participation,
                         double cap);

private:
    std::vector<CashFlow> cashFlows(
        const std::vector<double>& path) const override;

    double participation_{};
    double cap_{};
};
