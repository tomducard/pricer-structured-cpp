#pragma once
#include "AutocallBase.hpp"
#include <vector>

class StepDownAutocall : public AutocallBase {
public:
  StepDownAutocall(std::string underlying, std::vector<double> observationTimes,
                   double spot0, double notional, double couponRate,
                   std::vector<double> callBarriers, double protectionBarrier);

  double discountedPayoff(const std::vector<double> &path,
                          double riskFreeRate) const override;

private:
  std::vector<double> callBarriers_;
};