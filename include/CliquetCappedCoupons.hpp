#pragma once

#include "CliquetBase.hpp"

class CliquetCappedCoupons : public CliquetBase {
public:
    CliquetCappedCoupons(std::string underlying,
                         std::vector<double> observationTimes,
                         double spot0,
                         double notional,
                         double participation,
                         double cap);

protected:
    double payoffImpl(const std::vector<double>& path) const override;

private:
    double participation_{};
    double cap_{};
};