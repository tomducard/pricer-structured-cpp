#pragma once

#include "StructuredProduct.hpp"
#include <vector>
#include <string>

class AutocallBase : public StructuredProduct {
public:
    AutocallBase(std::string underlying,
                 std::vector<double> observationTimes,
                 double spot0,
                 double notional,
                 double couponRate,
                 double callBarrier,
                 double protectionBarrier);

    // Les enfants devront implémenter cashFlows directement
    // virtual std::vector<CashFlow> cashFlows(const std::vector<double>& path) const = 0;
    // ^ Déjà déclaré pur dans StructuredProduct

    const std::vector<double>& observationTimes() const override { return observationTimes_; }
    const std::string& underlying() const override { return underlying_; }

protected:
    double spot0() const { return spot0_; }
    double notional() const { return notional_; }
    double couponRate() const { return couponRate_; }
    double callBarrier() const { return callBarrier_; }
    double protectionBarrier() const { return protectionBarrier_; }
    const std::vector<double>& times() const { return observationTimes_; }

    virtual double terminalRedemption(double spotT) const;

private:
    std::string underlying_;
    std::vector<double> observationTimes_;
    double spot0_;
    double notional_;
    double couponRate_;
    double callBarrier_;
    double protectionBarrier_;
};