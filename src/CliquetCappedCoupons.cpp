#include "CliquetCappedCoupons.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

CliquetCappedCoupons::CliquetCappedCoupons(
    std::string underlying,
    std::vector<double> observationTimes,
    double spot0,
    double notional,
    double participation,
    double cap)
    : CliquetBase(std::move(underlying),
                  std::move(observationTimes),
                  spot0,
                  notional),
      participation_(participation),
      cap_(cap) {}

double CliquetCappedCoupons::payoffImpl(
    const std::vector<double>& path) const {
    if (path.empty()) {
        throw std::runtime_error("Cliquet path is empty");
    }
    if (spot0() <= 0.0) {
        return notional();
    }

    double couponSum = 0.0;
    double prevSpot = spot0();
    for (double spot : path) {
        if (prevSpot <= 0.0) {
            prevSpot = spot;
            continue;
        }
        const double ret = spot / prevSpot - 1.0;
        const double positiveReturn = std::max(ret, 0.0);
        const double participated = participation_ * positiveReturn;
        const double coupon = std::clamp(participated, 0.0, cap_);
        couponSum += coupon;
        prevSpot = spot;
    }

    return notional() * (1.0 + couponSum);
}
