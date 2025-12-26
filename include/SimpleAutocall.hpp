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

    std::vector<CashFlow> cashFlows(const std::vector<double>& path) const override;
};