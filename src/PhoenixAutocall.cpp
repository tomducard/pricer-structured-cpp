#include "PhoenixAutocall.hpp"
#include <algorithm>

PhoenixAutocall::PhoenixAutocall(std::string underlying,
                                 std::vector<double> observationTimes,
                                 double spot0, double notional,
                                 double couponRate, double callBarrier,
                                 double protectionBarrier, double couponBarrier)
    : AutocallBase(std::move(underlying), std::move(observationTimes), spot0,
                   notional, couponRate, callBarrier, protectionBarrier),
      couponBarrier_(couponBarrier) {}

double PhoenixAutocall::discountedPayoff(const std::vector<double> &path,
                                         double riskFreeRate) const {
  double totalValue = 0.0;
  const auto &obs = times();
  const std::size_t steps = std::min(path.size(), obs.size());

  for (std::size_t i = 0; i < steps; ++i) {
    // Coupon
    if (path[i] >= couponBarrier_) {
      totalValue +=
          (notional() * couponRate()) * std::exp(-riskFreeRate * obs[i]);
    }
    // Autocall
    if (path[i] >= callBarrier()) {
      totalValue += notional() * std::exp(-riskFreeRate * obs[i]);
      return totalValue;
    }
  }

  // Maturité
  const double finalSpot = (steps > 0) ? path[steps - 1] : spot0();
  totalValue +=
      terminalRedemption(finalSpot) * std::exp(-riskFreeRate * obs.back());
  return totalValue;
}