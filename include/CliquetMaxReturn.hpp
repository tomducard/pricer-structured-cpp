// Ratchet-style cliquet: pays max positive return vs spot0 at maturity.
#pragma once

#include "CliquetBase.hpp"

class CliquetMaxReturn : public CliquetBase {
public:
    CliquetMaxReturn(std::string underlying,
                     std::vector<double> observationTimes,
                     double spot0,
                     double notional);

private:
    std::vector<CashFlow> cashFlows(
        const std::vector<double>& path) const override;
};
