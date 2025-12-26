#include "SimpleAutocall.hpp"

#include <algorithm>
#include <stdexcept>

SimpleAutocall::SimpleAutocall(std::string underlying,
                               std::vector<double> observationTimes,
                               double spot0,
                               double notional,
                               double couponRate,
                               double callBarrier,
                               double protectionBarrier)
    : AutocallBase(std::move(underlying), std::move(observationTimes), spot0,
                   notional, couponRate, callBarrier, protectionBarrier) {}

std::pair<double, double> SimpleAutocall::payoffAndPayTimeImpl(
    const std::vector<double>& path) const {
    const auto& obs = times();
    const std::size_t steps = std::min(path.size(), obs.size());
    if (steps == 0) {
        throw std::runtime_error("path is empty");
    }

    for (std::size_t i = 0; i < steps; ++i) {
        if (path[i] >= callBarrier()) {
            return {notional() * (1.0 + couponRate()), obs[i]};
        }
    }

    const double finalSpot = path[steps - 1];
    const double payTime = obs.back();
    if (finalSpot >= protectionBarrier()) {
        return {notional(), payTime};
    }
    if (spot0() <= 0.0) {
        return {0.0, payTime};
    }
    return {notional() * (finalSpot / spot0()), payTime};
}
