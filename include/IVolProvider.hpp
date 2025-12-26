// Abstract volatility surface interface (plugged into path models).
#pragma once

class IVolProvider {
public:
    virtual ~IVolProvider() = default;

    virtual double vol(double expiry, double strike) const = 0;
};
