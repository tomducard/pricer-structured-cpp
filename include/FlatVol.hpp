// Trivial flat-vol implementation of IVolProvider.
#pragma once

#include "IVolProvider.hpp"

class FlatVol : public IVolProvider {
public:
    explicit FlatVol(double volatility);

    double vol(double expiry, double strike) const override;

private:
    double volatility_;
};
