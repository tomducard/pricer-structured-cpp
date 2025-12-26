#include "MemoryPhoenixAutocall.hpp"

#include <algorithm>
#include <stdexcept>

MemoryPhoenixAutocall::MemoryPhoenixAutocall(
    std::string underlying,
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

std::pair<double, double> MemoryPhoenixAutocall::payoffAndPayTimeImpl(
    const std::vector<double>& path) const {
    const auto& obs = times();
    const std::size_t steps = std::min(path.size(), obs.size());
    if (steps == 0) {
        throw std::runtime_error("path is empty");
    }

    double accruedCoupons = 0.0;
    double paidCoupons = 0.0;
    const double couponAmount = couponRate() * notional();

    for (std::size_t i = 0; i < steps; ++i) {
        accruedCoupons += couponAmount;

        if (path[i] >= couponBarrier_) {
            paidCoupons += accruedCoupons;
            accruedCoupons = 0.0;
        }

        if (path[i] >= callBarrier()) {
            paidCoupons += accruedCoupons;
            accruedCoupons = 0.0;
            return {notional() * (1.0 + couponRate()) + paidCoupons, obs[i]};
        }
    }

    const double finalSpot = path[steps - 1];
    const double payTime = obs.back();
    paidCoupons += accruedCoupons;
    if (finalSpot >= protectionBarrier()) {
        return {notional() + paidCoupons, payTime};
    }
    if (spot0() <= 0.0) {
        return {paidCoupons, payTime};
    }
    return {notional() * (finalSpot / spot0()) + paidCoupons, payTime};
}

