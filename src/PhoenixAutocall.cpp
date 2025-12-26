#include "PhoenixAutocall.hpp"

#include <algorithm>
#include <stdexcept>

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

std::pair<double, double> PhoenixAutocall::payoffAndPayTimeImpl(
    const std::vector<double>& path) const {
    const auto& obs = times();
    const std::size_t steps = std::min(path.size(), obs.size());
    if (steps == 0) {
        throw std::runtime_error("path is empty");
    }

    double couponAccrual = 0.0;
    for (std::size_t i = 0; i < steps; ++i) {
        if (path[i] >= couponBarrier_) {
            couponAccrual += couponRate() * notional();
        }

        if (path[i] >= callBarrier()) {
            return {notional() * (1.0 + couponRate()) + couponAccrual, obs[i]};
        }
    }

    const double finalSpot = path[steps - 1];
    const double payTime = obs.back();
    if (finalSpot >= protectionBarrier()) {
        return {notional() + couponAccrual, payTime};
    }
    if (spot0() <= 0.0) {
        return {couponAccrual, payTime};
    }
    return {notional() * (finalSpot / spot0()) + couponAccrual, payTime};
}

