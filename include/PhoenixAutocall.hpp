#pragma once
#include "AutocallBase.hpp"

class PhoenixAutocall : public AutocallBase {
public:
  PhoenixAutocall(std::string underlying, std::vector<double> observationTimes,
                  double spot0, double notional, double couponRate,
                  double callBarrier, double protectionBarrier,
                  double couponBarrier);

  double discountedPayoff(const std::vector<double> &path,
                          double riskFreeRate) const override;

private:
  double couponBarrier_{};
};