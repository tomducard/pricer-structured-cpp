#pragma once

#include <string>
#include <vector>

class StructuredProduct {
public:
  virtual ~StructuredProduct() = default;

  // Calcule directement le payoff total actualisé pour un chemin donné
  virtual double discountedPayoff(const std::vector<double> &path,
                                  double riskFreeRate) const = 0;

  virtual const std::vector<double> &observationTimes() const = 0;
  virtual const std::string &underlying() const = 0;
};