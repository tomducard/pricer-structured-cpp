#include "CliquetBase.hpp"
#include <utility>

CliquetBase::CliquetBase(std::string underlying,
                         std::vector<double> observationTimes, double spot0,
                         double notional)
    : underlying_(std::move(underlying)),
      observationTimes_(std::move(observationTimes)), spot0_(spot0),
      notional_(notional) {}

double CliquetBase::discountedPayoff(const std::vector<double> &path,
                                     double riskFreeRate) const {
  double amount = payoffImpl(path); // Appelle MaxReturn ou CappedCoupons
  double payTime = observationTimes_.empty() ? 0.0 : observationTimes_.back();
  return amount * std::exp(-riskFreeRate * payTime);
}

const std::vector<double> &CliquetBase::observationTimes() const {
  return observationTimes_;
}

const std::string &CliquetBase::underlying() const { return underlying_; }