#pragma once
#include "AutocallBase.hpp"

class AirbagAutocall : public AutocallBase {
public:
  AirbagAutocall(std::string underlying, std::vector<double> observationTimes,
                 double spot0, double notional, double couponRate,
                 double callBarrier, double protectionBarrier,
                 double airbagFloor);

  double discountedPayoff(const std::vector<double> &path,
                          double riskFreeRate) const override;

private:
  double terminalRedemption(double spotT) const override;
  double airbagFloor_{};
};