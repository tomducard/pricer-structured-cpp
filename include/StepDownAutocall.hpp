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

    std::vector<CashFlow> cashFlows(const std::vector<double>& path) const override;

private:
    std::vector<double> callBarriers_;
};