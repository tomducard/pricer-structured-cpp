#pragma once

#include <string>
#include <vector>

struct CashFlow {
    double amount;
    double time;
};

class StructuredProduct {
public:
    virtual ~StructuredProduct() = default;

    // Seule méthode requise : renvoyer la liste des flux
    virtual std::vector<CashFlow> cashFlows(const std::vector<double>& path) const = 0;

    virtual const std::vector<double>& observationTimes() const = 0;
    virtual const std::string& underlying() const = 0;
};