#include "PricerRunner.hpp"

#include "AirbagAutocall.hpp"
#include "MemoryPhoenixAutocall.hpp"
#include "PhoenixAutocall.hpp"
#include "SimpleAutocall.hpp"
#include "StepDownAutocall.hpp"
#include "CliquetCappedCoupons.hpp"
#include "CliquetMaxReturn.hpp"

#include "BlackScholesMC.hpp"
#include "HestonMC.hpp"
#include "MarketData.hpp"
#include "PathModel.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

namespace {
constexpr double kSpotBumpFraction = 0.005;
constexpr double kVolBumpAdd = 0.01;

// Factory helper to create the model with the correct parameters
std::unique_ptr<PathModelBase> makePathModel(const PricingInputs& inputs) {
    switch (inputs.modelType) {
    case ModelType::BlackScholes:
        // Pass sigma directly to the BS model
        return std::make_unique<BlackScholesMC>(inputs.sigma);
    case ModelType::Heston:
        return std::make_unique<HestonMC>(inputs.hestonV0, inputs.hestonKappa,
                                          inputs.hestonTheta, inputs.hestonXi,
                                          inputs.hestonRho);
    }
    return std::make_unique<BlackScholesMC>(inputs.sigma);
}

double runMonteCarlo(const StructuredProduct& product,
                     const MarketData& data,
                     const PathModelBase& model,
                     std::size_t paths,
                     unsigned int seed,
                     double& standardError) {
    const auto& times = product.observationTimes();
    // Retrieve spot from MarketData
    const auto& quote = data.getQuote(product.underlying());
    const double r = data.riskFreeRate();

    std::vector<double> immediatePath{quote.spot};

    if (times.empty()) {
        auto flows = product.cashFlows(immediatePath);
        double val = 0.0;
        for(const auto& flow : flows) {
            val += flow.amount * std::exp(-r * flow.time);
        }
        standardError = 0.0;
        return val;
    }

    std::mt19937 rng(seed);
    double payoffSum = 0.0;
    double payoffSqSum = 0.0;

    for (std::size_t i = 0; i < paths; ++i) {
        // The model uses quote.spot as the starting point
        const std::vector<double> path =
            model.simulatePath(quote.spot, times, data, rng);

        const std::vector<CashFlow> flows =
            product.cashFlows(path.empty() ? immediatePath : path);

        double pathValue = 0.0;
        for (const auto& flow : flows) {
            pathValue += flow.amount * std::exp(-r * flow.time);
        }

        payoffSum += pathValue;
        payoffSqSum += pathValue * pathValue;
    }

    const double n = static_cast<double>(paths);
    const double mean = payoffSum / n;
    const double numerator = payoffSqSum - n * mean * mean;
    const double sampleVariance = n > 1 ? std::max(numerator / (n - 1.0), 0.0)
                                        : 0.0;
    standardError = n > 0 ? std::sqrt(sampleVariance / n) : 0.0;
    return mean;
}
} // namespace

PricingResults priceAutocall(const PricingInputs& inputs) {
    MarketData marketData;
    marketData.setRiskFreeRate(inputs.rate);
    // Store spot and sigma in MarketData, even if BS uses its own sigma member now,
    // this is useful for consistency or if other components need it.
    marketData.setQuote(inputs.underlying,
                        MarketData::Quote{inputs.spot, inputs.sigma});

    // No need for setVolProvider here.

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

    // 1. Base price calculation
    const double price = runMonteCarlo(*product, marketData, *pathModel,
                                       inputs.paths, inputs.seed, stdError);

    // Bid/Ask
    const double spread = inputs.notional * inputs.spreadFraction;
    const double bid = price - spread;
    const double ask = price + spread;

    // 2. Delta calculation (Bump Spot)
    const double spotBumpSize = inputs.spot * kSpotBumpFraction;
    double delta = 0.0;
    if (spotBumpSize > 0.0) {
        MarketData spotUp = marketData;
        auto bumpedQuote = spotUp.getQuote(inputs.underlying);
        bumpedQuote.spot += spotBumpSize;
        spotUp.setQuote(inputs.underlying, bumpedQuote);

        double ignore = 0.0;
        // Note: The model remains the same (parameters unchanged), only MarketData changes (spot)
        const double bumpedPrice =
            runMonteCarlo(*product, spotUp, *pathModel, inputs.paths,
                          inputs.seed, ignore);
        delta = (bumpedPrice - price) / spotBumpSize;
    }

    // 3. Vega calculation (Bump Volatility)
    double vega = 0.0;
    double ignore = 0.0;

    if (inputs.modelType == ModelType::Heston) {
        // HESTON LOGIC: Shock the model parameters (v0)
        PricingInputs bumpedInputs = inputs;
        // Shock the initial variance.
        // Warning: kVolBumpAdd is intended for volatility (e.g., +1%).
        // To remain consistent, we can increase v0 significantly or
        // simply apply the bump as is if the user understands it is a sensitivity to v0.
        bumpedInputs.hestonV0 += kVolBumpAdd;

        auto vegaModel = makePathModel(bumpedInputs);
        const double vegaPrice = runMonteCarlo(*product, marketData, *vegaModel, inputs.paths, inputs.seed, ignore);

        vega = (vegaPrice - price) / kVolBumpAdd;

    } else {
        // BLACK-SCHOLES LOGIC: Shock the sigma
        PricingInputs bumpedInputs = inputs;
        bumpedInputs.sigma += kVolBumpAdd;

        auto vegaModel = makePathModel(bumpedInputs);

        // To ensure consistency, we also update MarketData
        // (although our new BSMC uses the internal sigma)
        MarketData volUp = marketData;
        auto q = volUp.getQuote(inputs.underlying);
        q.sigma += kVolBumpAdd;
        volUp.setQuote(inputs.underlying, q);

        const double vegaPrice = runMonteCarlo(*product, volUp, *vegaModel, inputs.paths, inputs.seed, ignore);

        vega = (vegaPrice - price) / kVolBumpAdd;
    }

    PricingResults results;
    results.price = price;
    results.stdError = stdError;
    results.delta = delta;
    results.vega = vega;
    results.bid = bid;
    results.ask = ask;
    return results;
}