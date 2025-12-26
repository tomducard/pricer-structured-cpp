#include "CliquetBase.hpp"

#include <algorithm>
#include <stdexcept>

CliquetBase::CliquetBase(std::string underlying,
                         std::vector<double> observationTimes,
                         double spot0,
                         double notional)
    : underlying_(std::move(underlying)),
      observationTimes_(std::move(observationTimes)),
      spot0_(spot0),
      notional_(notional) {}

double CliquetBase::payoff(const std::vector<double>& path) const {
    return payoffAndPayTime(path).first;
}

std::pair<double, double> CliquetBase::payoffAndPayTime(
    const std::vector<double>& path) const {
    if (observationTimes_.empty()) {
        throw std::runtime_error("Cliquet product requires observation times");
    }
    if (path.size() < observationTimes_.size()) {
        throw std::runtime_error("Cliquet path length mismatch");
    }
    const double payoffValue = payoffImpl(path);
    return {payoffValue, observationTimes_.back()};
}

double CliquetBase::terminalRedemption(double spotT) const {
    // For charting: assume a linear reference path to map ST -> payoff.
    if (observationTimes_.empty()) {
        return notional_;
    }
    const std::vector<double> path = buildReferencePath(spotT);
    return payoffImpl(path);
}

const std::vector<double>& CliquetBase::observationTimes() const {
    return observationTimes_;
}

const std::string& CliquetBase::underlying() const {
    return underlying_;
}

std::vector<double> CliquetBase::buildReferencePath(double terminalSpot) const {
    const std::size_t steps = observationTimes_.size();
    std::vector<double> path(steps);
    if (steps == 0) {
        return path;
    }
    if (steps == 1) {
        path[0] = terminalSpot;
        return path;
    }

    for (std::size_t i = 0; i < steps; ++i) {
        const double ratio =
            static_cast<double>(i) / static_cast<double>(steps - 1);
        path[i] = spot0_ + (terminalSpot - spot0_) * ratio;
    }
    return path;
}
