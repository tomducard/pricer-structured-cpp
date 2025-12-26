// Phoenix autocall: accrues coupons above coupon barrier; calls on barrier breach.
#pragma once

#include "AutocallBase.hpp"

class PhoenixAutocall : public AutocallBase {
public:
    PhoenixAutocall(std::string underlying,
                    std::vector<double> observationTimes,
                    double spot0,
                    double notional,
                    double couponRate,
                    double callBarrier,
                    double protectionBarrier,
                    double couponBarrier);

private:
    std::pair<double, double> payoffAndPayTimeImpl(
        const std::vector<double>& path) const override;

    double couponBarrier_{};
};
