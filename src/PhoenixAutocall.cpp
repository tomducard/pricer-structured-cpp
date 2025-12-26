#include "PhoenixAutocall.hpp"
#include <algorithm>

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
        // Coupon
        if (path[i] >= couponBarrier_) {
            flows.push_back({notional() * couponRate(), obs[i]});
        }
        // Autocall
        if (path[i] >= callBarrier()) {
            flows.push_back({notional(), obs[i]});
            return flows;
        }
    }

    // Maturité
    const double finalSpot = (steps > 0) ? path[steps - 1] : spot0();
    flows.push_back({terminalRedemption(finalSpot), obs.back()});
    return flows;
}