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

    std::vector<CashFlow> cashFlows(const std::vector<double>& path) const override;

private:
    double couponBarrier_{};
};