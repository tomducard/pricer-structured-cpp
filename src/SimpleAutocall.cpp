#include "SimpleAutocall.hpp"
#include <algorithm>
#include <vector>

SimpleAutocall::SimpleAutocall(std::string underlying,
                               std::vector<double> observationTimes,
                               double spot0,
                               double notional,
                               double couponRate,
                               double callBarrier,
                               double protectionBarrier)
    : AutocallBase(std::move(underlying), std::move(observationTimes), spot0,
                   notional, couponRate, callBarrier, protectionBarrier) {}

std::vector<CashFlow> SimpleAutocall::cashFlows(const std::vector<double>& path) const {
    std::vector<CashFlow> flows;
    const auto& obs = times();
    const std::size_t steps = std::min(path.size(), obs.size());

    for (std::size_t i = 0; i < steps; ++i) {
        // Condition d'Autocall (Rappel anticipé)
        if (path[i] >= callBarrier()) {
            // On paie le Nominal + le Coupon fixe
            flows.push_back({notional() * (1.0 + couponRate()), obs[i]});
            return flows; // Le produit s'arrête ici
        }
    }

    // Si on arrive à maturité sans rappel
    const double finalSpot = (steps > 0) ? path[steps - 1] : spot0();
    const double maturityTime = obs.back();

    // Remboursement final (géré par la fonction helper de la classe de base ou manuellement)
    flows.push_back({terminalRedemption(finalSpot), maturityTime});

    return flows;
}