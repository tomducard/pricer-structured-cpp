#include "PhoenixAutocall.hpp"
#include <algorithm>
#include <vector>

PhoenixAutocall::PhoenixAutocall(std::string underlying,
                                 std::vector<double> observationTimes,
                                 double spot0,
                                 double notional,
                                 double couponRate,
                                 double callBarrier,
                                 double protectionBarrier,
                                 double couponBarrier)
    : AutocallBase(std::move(underlying), std::move(observationTimes), spot0,
                   notional, couponRate, callBarrier, protectionBarrier),
      couponBarrier_(couponBarrier) {}

std::vector<CashFlow> PhoenixAutocall::cashFlows(const std::vector<double>& path) const {
    std::vector<CashFlow> flows;
    const auto& obs = times();
    const std::size_t steps = std::min(path.size(), obs.size());

    for (std::size_t i = 0; i < steps; ++i) {
        // 1. Coupon conditionnel (Phoenix)
        if (path[i] >= couponBarrier_) {
            flows.push_back({notional() * couponRate(), obs[i]});
        }

        // 2. Autocall
        if (path[i] >= callBarrier()) {
            flows.push_back({notional(), obs[i]});
            return flows; // Le produit s'arrête
        }
    }

    // 3. Maturité (Protection Capital ou PDI)
    const double finalSpot = (steps > 0) ? path[steps - 1] : spot0();
    const double maturityTime = obs.back();

    // Note : On utilise la méthode helper de la classe de base pour le remboursement final
    flows.push_back({terminalRedemption(finalSpot), maturityTime});

    return flows;
}