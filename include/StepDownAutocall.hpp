// Step-down autocall: date-specific call barriers; coupon paid only if called.
#pragma once

#include "AutocallBase.hpp"

#include <vector>

class StepDownAutocall : public AutocallBase {
public:
    StepDownAutocall(std::string underlying,
                     std::vector<double> observationTimes,
                     double spot0,
                     double notional,
                     double couponRate,
                     std::vector<double> callBarriers,
                     double protectionBarrier);

private:
    std::pair<double, double> payoffAndPayTimeImpl(
        const std::vector<double>& path) const override;

    std::vector<double> callBarriers_;
};
