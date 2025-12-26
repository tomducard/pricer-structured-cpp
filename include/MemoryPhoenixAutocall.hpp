// Memory phoenix: unpaid coupons accrue until a coupon barrier hit or call.
#pragma once

#include "AutocallBase.hpp"

class MemoryPhoenixAutocall : public AutocallBase {
public:
    MemoryPhoenixAutocall(std::string underlying,
                          std::vector<double> observationTimes,
                          double spot0,
                          double notional,
                          double couponRate,
                          double callBarrier,
                          double protectionBarrier,
                          double couponBarrier);

private:
    std::vector<CashFlow> cashFlows(
        const std::vector<double>& path) const override;

    double couponBarrier_{};
};
