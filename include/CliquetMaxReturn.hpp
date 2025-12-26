#pragma once

#include "CliquetBase.hpp"

class CliquetMaxReturn : public CliquetBase {
public:
    CliquetMaxReturn(std::string underlying,
                     std::vector<double> observationTimes,
                     double spot0,
                     double notional);

protected:
    // On implémente la logique spécifique ici, appelée par CliquetBase::cashFlows
    double payoffImpl(const std::vector<double>& path) const override;
};