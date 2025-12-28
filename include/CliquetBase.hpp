#pragma once
#include "StructuredProduct.hpp"
#include <string>
#include <vector>

class CliquetBase : public StructuredProduct {
public:
  CliquetBase(std::string underlying, std::vector<double> observationTimes,
              double spot0, double notional);

  // Adaptation : Les cliquets renvoient un flux unique via discountedPayoff
  double discountedPayoff(const std::vector<double> &path,
                          double riskFreeRate) const override final;

  const std::vector<double> &observationTimes() const override;
  const std::string &underlying() const override;

protected:
  const std::vector<double> &times() const { return observationTimes_; }
  double spot0() const { return spot0_; }
  double notional() const { return notional_; }

  // Méthode interne pour calculer le montant final
  virtual double payoffImpl(const std::vector<double> &path) const = 0;

private:
  std::string underlying_;
  std::vector<double> observationTimes_;
  double spot0_{};
  double notional_{};
};