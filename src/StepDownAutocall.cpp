#include "StepDownAutocall.hpp"
#include <algorithm>
#include <utility>

StepDownAutocall::StepDownAutocall(std::string underlying,
                                   std::vector<double> observationTimes,
                                   double spot0, double notional,
                                   double couponRate,
                                   std::vector<double> callBarriers,
                                   double protectionBarrier)
    : AutocallBase(std::move(underlying), std::move(observationTimes), spot0,
                   notional, couponRate,
                   callBarriers.empty() ? 0.0 : callBarriers.front(),
                   protectionBarrier),
      callBarriers_(std::move(callBarriers)) {}

double StepDownAutocall::discountedPayoff(const std::vector<double> &path,
                                          double riskFreeRate) const {
  const auto &obs = times();
  const std::size_t steps = std::min(path.size(), obs.size());

  for (std::size_t i = 0; i < steps; ++i) {
    double currentBarrier =
        callBarriers_.empty()
            ? callBarrier()
            : callBarriers_[std::min(i, callBarriers_.size() - 1)];

    if (path[i] >= currentBarrier) {
      double amount = notional() * (1.0 + couponRate());
      return amount * std::exp(-riskFreeRate * obs[i]);
    }
  }

  const double finalSpot = (steps > 0) ? path[steps - 1] : spot0();
  double amount = terminalRedemption(finalSpot);
  return amount * std::exp(-riskFreeRate * obs.back());
}