#include "AutocallBase.hpp"

AutocallBase::AutocallBase(std::string underlying,
                           std::vector<double> observationTimes,
                           double spot0,
                           double notional,
                           double couponRate,
                           double callBarrier,
                           double protectionBarrier)
    : StructuredProduct(std::move(underlying), std::move(observationTimes)),
      notional_(notional), couponRate_(couponRate),
      callBarrier_(callBarrier), protectionBarrier_(protectionBarrier),
      spot0_(spot0) {}

double AutocallBase::terminalRedemption(double finalSpot) const {
    // If the final spot is above the protection barrier,
    // capital is guaranteed (plus potentially the last coupon, handled in derived classes).
    if (finalSpot >= protectionBarrier_) {
        return notional_;
    }
    // Capital at risk: The investor loses money proportional to the spot drop.
    return notional_ * (finalSpot / spot0_);
}