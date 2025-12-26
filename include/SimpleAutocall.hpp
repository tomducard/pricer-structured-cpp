// Basic autocall: single barrier, coupon paid only if called.
#pragma once

#include "AutocallBase.hpp"

class SimpleAutocall : public AutocallBase {
public:
    SimpleAutocall(std::string underlying,
                   std::vector<double> observationTimes,
                   double spot0,
                   double notional,
                   double couponRate,
                   double callBarrier,
                   double protectionBarrier);

private:
    std::vector<CashFlow> cashFlows(
        const std::vector<double>& path) const override;
};
