#include "PricerRunner.hpp"

#include "AirbagAutocall.hpp"
#include "MemoryPhoenixAutocall.hpp"
#include "PhoenixAutocall.hpp"
#include "SimpleAutocall.hpp"
#include "StepDownAutocall.hpp"
#include "CliquetCappedCoupons.hpp"
#include "CliquetMaxReturn.hpp"

#include "BlackScholesMC.hpp"
#include "FlatVol.hpp"
#include "HestonMC.hpp"
#include "MarketData.hpp"
#include "PathModel.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

namespace {
// Small shocks used to compute finite-difference delta and vega.
constexpr double kSpotBumpFraction = 0.005;
constexpr double kVolBumpAdd = 0.01;

std::unique_ptr<PathModelBase> makePathModel(const PricingInputs& inputs) {
    switch (inputs.modelType) {
    case ModelType::BlackScholes:
        return std::make_unique<BlackScholesMC>();
    case ModelType::Heston:
        return std::make_unique<HestonMC>(inputs.hestonV0, inputs.hestonKappa,
                                          inputs.hestonTheta, inputs.hestonXi,
                                          inputs.hestonRho);
    }
    return std::make_unique<BlackScholesMC>();
}

double runMonteCarlo(const StructuredProduct& product,
                     const MarketData& data,
                     const PathModelBase& model,
                     std::size_t paths,
                     unsigned int seed,
                     double& standardError) {
    // Generic MC loop: simulate paths, get (payoff,time) from product, discount,
    // and accumulate mean/standard error.
    const auto& times = product.observationTimes();
    const auto& quote = data.getQuote(product.underlying());
    const double r = data.riskFreeRate();

    std::vector<double> immediatePath{quote.spot};
    if (times.empty()) {
        const auto result = product.payoffAndPayTime(immediatePath);
        standardError = 0.0;
        return std::exp(-r * result.second) * result.first;
    }

    std::mt19937 rng(seed);
    double payoffSum = 0.0;
    double payoffSqSum = 0.0;
    for (std::size_t i = 0; i < paths; ++i) {
        const std::vector<double> path =
            model.simulatePath(quote.spot, times, data, rng);
        const auto [payoff, payTime] =
            product.payoffAndPayTime(path.empty() ? immediatePath : path);
        const double discounted = std::exp(-r * payTime) * payoff;
        payoffSum += discounted;
        payoffSqSum += discounted * discounted;
    }

    const double n = static_cast<double>(paths);
    const double mean = payoffSum / n;
    const double numerator = payoffSqSum - n * mean * mean;
    const double sampleVariance = n > 1 ? std::max(numerator / (n - 1.0), 0.0)
                                        : 0.0;
    standardError =
        n > 0 ? std::sqrt(sampleVariance / n) : 0.0;
    return mean;
}
} // namespace

PricingResults priceAutocall(const PricingInputs& inputs) {
    MarketData marketData;
    marketData.setRiskFreeRate(inputs.rate);
    marketData.setQuote(inputs.underlying,
                        MarketData::Quote{inputs.spot, inputs.sigma});
    marketData.setVolProvider(std::make_shared<FlatVol>(inputs.sigma));

    // Build the requested product (autocall or cliquet) from inputs.
    std::unique_ptr<StructuredProduct> product;
    if (inputs.productFamily == ProductFamily::Autocall) {
        switch (inputs.autocallType) {
        case AutocallType::Simple:
            product = std::make_unique<SimpleAutocall>(
                inputs.underlying, inputs.observationTimes, inputs.spot,
                inputs.notional, inputs.coupon, inputs.autocallBarrier,
                inputs.protectionBarrier);
            break;
        case AutocallType::Phoenix:
            product = std::make_unique<PhoenixAutocall>(
                inputs.underlying, inputs.observationTimes, inputs.spot,
                inputs.notional, inputs.coupon, inputs.autocallBarrier,
                inputs.protectionBarrier, inputs.couponBarrier);
            break;
        case AutocallType::MemoryPhoenix:
            product = std::make_unique<MemoryPhoenixAutocall>(
                inputs.underlying, inputs.observationTimes, inputs.spot,
                inputs.notional, inputs.coupon, inputs.autocallBarrier,
                inputs.protectionBarrier, inputs.couponBarrier);
            break;
        case AutocallType::StepDown: {
            std::vector<double> schedule = inputs.callBarriers;
            if (schedule.empty()) {
                schedule.assign(inputs.observationTimes.size(),
                                inputs.autocallBarrier);
            }
            product = std::make_unique<StepDownAutocall>(
                inputs.underlying, inputs.observationTimes, inputs.spot,
                inputs.notional, inputs.coupon, schedule,
                inputs.protectionBarrier);
            break;
        }
        case AutocallType::Airbag:
            product = std::make_unique<AirbagAutocall>(
                inputs.underlying, inputs.observationTimes, inputs.spot,
                inputs.notional, inputs.coupon, inputs.autocallBarrier,
                inputs.protectionBarrier, inputs.airbagFloor);
            break;
        }
    } else {
        switch (inputs.cliquetType) {
        case CliquetType::MaxReturn:
            product = std::make_unique<CliquetMaxReturn>(
                inputs.underlying, inputs.observationTimes, inputs.spot,
                inputs.notional);
            break;
        case CliquetType::CappedCoupons:
            product = std::make_unique<CliquetCappedCoupons>(
                inputs.underlying, inputs.observationTimes, inputs.spot,
                inputs.notional, inputs.cliquetParticipation,
                inputs.cliquetCap);
            break;
        }
    }

    double stdError = 0.0;
    auto pathModel = makePathModel(inputs);
    const double price = runMonteCarlo(*product, marketData, *pathModel,
                                       inputs.paths, inputs.seed, stdError);

    // Simple transaction cost layer: bid/ask around mid.
    const double spread = inputs.notional * inputs.spreadFraction;
    const double bid = price - spread;
    const double ask = price + spread;

    // Finite-difference delta (re-uses same RNG seed).
    const double spotBumpSize = inputs.spot * kSpotBumpFraction;
    double delta = 0.0;
    if (spotBumpSize > 0.0) {
        MarketData spotUp = marketData;
        auto bumpedQuote = spotUp.getQuote(inputs.underlying);
        bumpedQuote.spot += spotBumpSize;
        spotUp.setQuote(inputs.underlying, bumpedQuote);
        double ignore = 0.0;
        const double bumpedPrice =
            runMonteCarlo(*product, spotUp, *pathModel, inputs.paths,
                          inputs.seed, ignore);
        delta = (bumpedPrice - price) / spotBumpSize;
    }

    MarketData volUp = marketData;
    volUp.setVolProvider(
        std::make_shared<FlatVol>(inputs.sigma + kVolBumpAdd));
    double ignore = 0.0;
    const double vegaPrice =
        runMonteCarlo(*product, volUp, *pathModel, inputs.paths,
                      inputs.seed, ignore);
    const double vega = (vegaPrice - price) / kVolBumpAdd;

    PricingResults results;
    results.price = price;
    results.stdError = stdError;
    results.delta = delta;
    results.vega = vega;
    results.bid = bid;
    results.ask = ask;
    return results;
}
