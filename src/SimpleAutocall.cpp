#include "SimpleAutocall.hpp"
#include <algorithm>
#include <vector>

SimpleAutocall::SimpleAutocall(std::string underlying,
                               std::vector<double> observationTimes,
                               double spot0, double notional, double couponRate,
                               double callBarrier, double protectionBarrier)
    : AutocallBase(std::move(underlying), std::move(observationTimes), spot0,
                   notional, couponRate, callBarrier, protectionBarrier) {}

double SimpleAutocall::discountedPayoff(const std::vector<double> &path,
                                        double riskFreeRate) const {
  const auto &obs = times();
  const std::size_t steps = std::min(path.size(), obs.size());

  for (std::size_t i = 0; i < steps; ++i) {
    if (path[i] >= callBarrier()) {
      // Autocall : Nominal + Coupon
      double amount = notional() * (1.0 + couponRate());
      return amount * std::exp(-riskFreeRate * obs[i]);
    }
  }

  const double finalSpot = (steps > 0) ? path[steps - 1] : spot0();
  double amount = terminalRedemption(finalSpot);
  return amount * std::exp(-riskFreeRate * obs.back());
}