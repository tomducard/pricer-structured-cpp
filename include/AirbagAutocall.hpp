// Airbag autocall: standard call logic with a minimum redemption floor at maturity.
#pragma once

#include "AutocallBase.hpp"

class AirbagAutocall : public AutocallBase {
public:
    AirbagAutocall(std::string underlying,
                   std::vector<double> observationTimes,
                   double spot0,
                   double notional,
                   double couponRate,
                   double callBarrier,
                   double protectionBarrier,
                   double airbagFloor);

private:
    std::vector<CashFlow> cashFlows(
        const std::vector<double>& path) const override;
    double terminalRedemption(double spotT) const override;

    double airbagFloor_{};
};
