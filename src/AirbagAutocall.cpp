#include "AirbagAutocall.hpp"

#include <algorithm>
#include <stdexcept>

AirbagAutocall::AirbagAutocall(std::string underlying,
                               std::vector<double> observationTimes,
                               double spot0,
                               double notional,
                               double couponRate,
                               double callBarrier,
                               double protectionBarrier,
                               double airbagFloor)
    : AutocallBase(std::move(underlying), std::move(observationTimes), spot0,
                   notional, couponRate, callBarrier, protectionBarrier),
      airbagFloor_(airbagFloor) {}

std::pair<double, double> AirbagAutocall::payoffAndPayTimeImpl(
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
        return {notional() * airbagFloor_, payTime};
    }
    const double ratio = finalSpot / spot0();
    const double adjusted = std::max(airbagFloor_, ratio);
    return {notional() * adjusted, payTime};
}

double AirbagAutocall::terminalRedemption(double spotT) const {
    const double base = AutocallBase::terminalRedemption(spotT);
    const double minRedemption = notional() * airbagFloor_;
    return std::max(base, minRedemption);
}
