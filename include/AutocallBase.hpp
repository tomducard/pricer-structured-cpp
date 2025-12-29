#pragma once
#include "StructuredProduct.hpp"
#include <vector>
#include <string>

/**
 * @brief Abstract base class for all Autocall products.
 *
 * Stores common parameters like barriers, coupon rates, and observation times.
 */
class AutocallBase : public StructuredProduct {
public:
    AutocallBase(std::string underlying,
                 std::vector<double> observationTimes,
                 double spot0,
                 double notional,
                 double couponRate,
                 double callBarrier,
                 double protectionBarrier);

    double notional() const { return notional_; }
    double couponRate() const { return couponRate_; }
    double callBarrier() const { return callBarrier_; }
    double protectionBarrier() const { return protectionBarrier_; }
    double spot0() const { return spot0_; }

protected:
    /**
     * @brief Calculates the terminal redemption amount at maturity.
     * @param finalSpot The spot price at the final observation.
     * @return The cash amount returned to the investor.
     */
    double terminalRedemption(double finalSpot) const;

private:
    double notional_;
    double couponRate_;
    double callBarrier_;
    double protectionBarrier_;
    double spot0_;
};