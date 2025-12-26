// Public-facing pricing inputs/results plus product/model enums used by the runner.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum class ProductFamily { Autocall, Cliquet };
enum class AutocallType { Simple, Phoenix, MemoryPhoenix, StepDown, Airbag };
enum class CliquetType { MaxReturn, CappedCoupons };
enum class ModelType { BlackScholes, Heston };

struct PricingInputs {
    std::string underlying{"SPX"};
    double spot{4000.0};
    double sigma{0.20};
    double rate{0.02};
    double notional{1000.0};
    double coupon{0.05};
    double autocallBarrier{4100.0};
    double protectionBarrier{3200.0};
    std::vector<double> observationTimes{0.25, 0.5, 0.75, 1.0};
    std::size_t paths{20000};
    unsigned int seed{1337};
    double spreadFraction{0.005};
    ProductFamily productFamily{ProductFamily::Autocall};
    AutocallType autocallType{AutocallType::Simple};
    CliquetType cliquetType{CliquetType::MaxReturn};
    ModelType modelType{ModelType::BlackScholes};
    double couponBarrier{4100.0};
    std::vector<double> callBarriers;
    double airbagFloor{0.7};
    double hestonV0{0.04};
    double hestonKappa{1.5};
    double hestonTheta{0.04};
    double hestonXi{0.5};
    double hestonRho{-0.5};
    double cliquetParticipation{1.0};
    double cliquetCap{0.05};
};

struct PricingResults {
    double price{};
    double stdError{};
    double delta{};
    double vega{};
    double bid{};
    double ask{};
};

PricingResults priceAutocall(const PricingInputs& inputs);
