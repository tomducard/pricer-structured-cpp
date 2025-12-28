#include "PricerRunner.hpp"

#include "AirbagAutocall.hpp"
#include "CliquetCappedCoupons.hpp"
#include "CliquetMaxReturn.hpp"
#include "MemoryPhoenixAutocall.hpp"
#include "PhoenixAutocall.hpp"
#include "SimpleAutocall.hpp"
#include "StepDownAutocall.hpp"

#include "BlackScholesMC.hpp"
// #include "FlatVol.hpp"  <-- SUPPRIMÉ
#include "HestonMC.hpp"
#include "MarketData.hpp"
#include "PathModel.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

namespace {
constexpr double kSpotBumpFraction = 0.005;
constexpr double kVolBumpAdd = 0.01;

// Factory helper pour créer le modèle avec les bons paramètres
std::unique_ptr<PathModelBase> makePathModel(const PricingInputs &inputs) {
  switch (inputs.modelType) {
  case ModelType::BlackScholes:
    // On passe sigma directement au modèle BS
    return std::make_unique<BlackScholesMC>(inputs.sigma);
  case ModelType::Heston:
    return std::make_unique<HestonMC>(inputs.hestonV0, inputs.hestonKappa,
                                      inputs.hestonTheta, inputs.hestonXi,
                                      inputs.hestonRho);
  }
  return std::make_unique<BlackScholesMC>(inputs.sigma);
}

double runMonteCarlo(const StructuredProduct &product, const MarketData &data,
                     const PathModelBase &model, std::size_t paths,
                     unsigned int seed, double &standardError) {
  const auto &times = product.observationTimes();
  // On récupère le spot depuis MarketData
  const auto &quote = data.getQuote(product.underlying());
  const double r = data.riskFreeRate();

  std::vector<double> immediatePath{quote.spot};

  if (times.empty()) {
    double val = product.discountedPayoff(immediatePath, r);
    standardError = 0.0;
    return val;
  }

  std::mt19937 rng(seed);
  double payoffSum = 0.0;
  double payoffSqSum = 0.0;

  for (std::size_t i = 0; i < paths; ++i) {
    // Le modèle utilise quote.spot comme point de départ
    const std::vector<double> path =
        model.simulatePath(quote.spot, times, data, rng);

    // NOUVEAU : Calcul direct du payoff actualisé
    double pathValue =
        product.discountedPayoff(path.empty() ? immediatePath : path, r);

    payoffSum += pathValue;
    payoffSqSum += pathValue * pathValue;
  }

  const double n = static_cast<double>(paths);
  const double mean = payoffSum / n;
  const double numerator = payoffSqSum - n * mean * mean;
  const double sampleVariance =
      n > 1 ? std::max(numerator / (n - 1.0), 0.0) : 0.0;
  standardError = n > 0 ? std::sqrt(sampleVariance / n) : 0.0;
  return mean;
}
} // namespace

PricingResults priceAutocall(const PricingInputs &inputs) {
  MarketData marketData;
  marketData.setRiskFreeRate(inputs.rate);
  // On stocke spot et sigma dans MarketData, même si BS utilise son propre
  // membre sigma désormais, c'est utile pour la cohérence ou si d'autres
  // composants en ont besoin.
  marketData.setQuote(inputs.underlying,
                      MarketData::Quote{inputs.spot, inputs.sigma});

  // Plus besoin de setVolProvider here.

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
        schedule.assign(inputs.observationTimes.size(), inputs.autocallBarrier);
      }
      product = std::make_unique<StepDownAutocall>(
          inputs.underlying, inputs.observationTimes, inputs.spot,
          inputs.notional, inputs.coupon, schedule, inputs.protectionBarrier);
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
          inputs.notional, inputs.cliquetParticipation, inputs.cliquetCap);
      break;
    }
  }

  double stdError = 0.0;
  auto pathModel = makePathModel(inputs);

  // 1. Calcul du prix de base
  const double price = runMonteCarlo(*product, marketData, *pathModel,
                                     inputs.paths, inputs.seed, stdError);

  // Bid/Ask
  const double spread = inputs.notional * inputs.spreadFraction;
  const double bid = price - spread;
  const double ask = price + spread;

  // 2. Calcul du Delta (Bump Spot)
  const double spotBumpSize = inputs.spot * kSpotBumpFraction;
  double delta = 0.0;
  if (spotBumpSize > 0.0) {
    MarketData spotUp = marketData;
    auto bumpedQuote = spotUp.getQuote(inputs.underlying);
    bumpedQuote.spot += spotBumpSize;
    spotUp.setQuote(inputs.underlying, bumpedQuote);

    double ignore = 0.0;
    // Note: Le modèle reste le même (paramètres inchangés), seul marketData
    // change (spot)
    const double bumpedPrice = runMonteCarlo(*product, spotUp, *pathModel,
                                             inputs.paths, inputs.seed, ignore);
    delta = (bumpedPrice - price) / spotBumpSize;
  }

  // 3. Calcul du Vega (Bump Volatilité)
  double vega = 0.0;
  double ignore = 0.0;

  if (inputs.modelType == ModelType::Heston) {
    // LOGIQUE HESTON : On choque les paramètres du modèle (v0)
    PricingInputs bumpedInputs = inputs;
    // On choque la variance initiale.
    // Attention: kVolBumpAdd est prévu pour la volatilité (ex: +1%).
    // Pour rester cohérent, on peut augmenter v0 de manière significative ou
    // simplement appliquer le bump tel quel si l'utilisateur comprend que c'est
    // une sensibilité à v0.
    bumpedInputs.hestonV0 += kVolBumpAdd;

    auto vegaModel = makePathModel(bumpedInputs);
    const double vegaPrice = runMonteCarlo(*product, marketData, *vegaModel,
                                           inputs.paths, inputs.seed, ignore);

    vega = (vegaPrice - price) / kVolBumpAdd;

  } else {
    // LOGIQUE BLACK-SCHOLES : On choque le sigma
    PricingInputs bumpedInputs = inputs;
    bumpedInputs.sigma += kVolBumpAdd;

    auto vegaModel = makePathModel(bumpedInputs);

    // Pour être sûr que tout soit cohérent, on met aussi à jour marketData
    // (bien que notre nouveau BSMC utilise le sigma interne)
    MarketData volUp = marketData;
    auto q = volUp.getQuote(inputs.underlying);
    q.sigma += kVolBumpAdd;
    volUp.setQuote(inputs.underlying, q);

    const double vegaPrice = runMonteCarlo(*product, volUp, *vegaModel,
                                           inputs.paths, inputs.seed, ignore);

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