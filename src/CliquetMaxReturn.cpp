#include "CliquetMaxReturn.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

CliquetMaxReturn::CliquetMaxReturn(std::string underlying,
                                   std::vector<double> observationTimes,
                                   double spot0,
                                   double notional)
    : CliquetBase(std::move(underlying),
                  std::move(observationTimes),
                  spot0,
                  notional) {}

double CliquetMaxReturn::payoffImpl(const std::vector<double>& path) const {
    if (path.empty()) {
        throw std::runtime_error("Cliquet path is empty");
    }
    if (spot0() <= 0.0) {
        return 0.0;
    }

    double maxReturn = 0.0;
    for (double spot : path) {
        const double ratio = spot / spot0() - 1.0;
        maxReturn = std::max(maxReturn, ratio);
    }

    return notional() * std::max(maxReturn, 0.0);
}
