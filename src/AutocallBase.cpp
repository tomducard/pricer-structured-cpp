#include "AutocallBase.hpp"

#include <stdexcept>
#include <utility>

AutocallBase::AutocallBase(std::string underlying,
                           std::vector<double> observationTimes,
                           double spot0,
                           double notional,
                           double couponRate,
                           double callBarrier,
                           double protectionBarrier)
    : underlying_(std::move(underlying)),
      observationTimes_(std::move(observationTimes)),
      spot0_(spot0),
      notional_(notional),
      couponRate_(couponRate),
      callBarrier_(callBarrier),
      protectionBarrier_(protectionBarrier) {
    if (observationTimes_.empty()) {
        throw std::invalid_argument("Autocall requires observation dates");
    }
}

double AutocallBase::payoff(const std::vector<double>& path) const {
    return payoffAndPayTime(path).first;
}

const std::vector<double>& AutocallBase::observationTimes() const {
    return observationTimes_;
}

const std::string& AutocallBase::underlying() const {
    return underlying_;
}

std::pair<double, double> AutocallBase::payoffAndPayTime(
    const std::vector<double>& path) const {
    return payoffAndPayTimeImpl(path);
}

double AutocallBase::terminalRedemption(double spotT) const {
    // Plain maturity rule: protect to barrier, otherwise capital-at-risk vs spot0.
    if (spotT >= protectionBarrier_) {
        return notional_;
    }
    if (spot0_ <= 0.0) {
        return 0.0;
    }
    return notional_ * (spotT / spot0_);
}
