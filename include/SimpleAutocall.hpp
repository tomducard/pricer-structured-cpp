#pragma once
#include "AutocallBase.hpp"

class SimpleAutocall : public AutocallBase {
public:
  SimpleAutocall(std::string underlying, std::vector<double> observationTimes,
                 double spot0, double notional, double couponRate,
                 double callBarrier, double protectionBarrier);

  double discountedPayoff(const std::vector<double> &path,
                          double riskFreeRate) const override;
};