// Core interface every structured product must implement (payoff, timing, curve).
#pragma once

#include <string>
#include <vector>

// Représente un flux financier (montant et date de paiement)
struct CashFlow {
    double amount;
    double time;
};

class StructuredProduct {
public:
    virtual ~StructuredProduct() = default;

    // Renvoie la liste complète des flux générés par un scénario de marché
    virtual std::vector<CashFlow> cashFlows(const std::vector<double>& path) const = 0;

    virtual const std::vector<double>& observationTimes() const = 0;
    virtual const std::string& underlying() const = 0;
};