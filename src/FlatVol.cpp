#include "FlatVol.hpp"

FlatVol::FlatVol(double volatility) : volatility_(volatility) {}

double FlatVol::vol(double /*expiry*/, double /*strike*/) const {
    return volatility_;
}
