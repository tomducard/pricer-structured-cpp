#include "AirbagAutocall.hpp"
#include "CliquetCappedCoupons.hpp"
#include "CliquetMaxReturn.hpp"
#include "InputUtils.hpp"
#include "MemoryPhoenixAutocall.hpp"
#include "PhoenixAutocall.hpp"
#include "PricerRunner.hpp"
#include "SimpleAutocall.hpp"
#include "StepDownAutocall.hpp"
#include "StructuredProduct.hpp"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPen>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QChart>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include <algorithm> // Ajout nécessaire pour std::max
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

// Minimal Qt window: handles user inputs, instantiates the chosen
// product/model, runs pricing via PricerRunner, and renders a simple payoff
// chart.
class PricerWindow : public QWidget {
  Q_OBJECT

public:
  PricerWindow(QWidget *parent = nullptr);

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void handlePrice();

private:
  static QString doubleToQString(double value);
  static QString sizeToQString(std::size_t value);
  static QString uintToQString(unsigned int value);

  double readDouble(QLineEdit *edit, double fallback) const;
  std::size_t readSizeT(QLineEdit *edit, std::size_t fallback) const;
  unsigned int readUInt(QLineEdit *edit, unsigned int fallback) const;

  void updateResults(const PricingResults &results);
  void showError(const QString &message);
  PricingInputs gatherInputs() const;
  void updatePayoffChart();
  void connectInputField(QLineEdit *edit);
  void connectInputs();
  std::vector<double> defaultCallBarrierList() const;
  std::unique_ptr<StructuredProduct>
  createProduct(const PricingInputs &inputs) const;
  void loadSettings();
  void saveSettings() const;

  PricingInputs defaults_;
  QWidget *inputContainer_{};
  QScrollArea *inputScroll_{};

  QComboBox *familyCombo_{};
  QComboBox *autocallCombo_{};
  QComboBox *cliquetCombo_{};
  QComboBox *modelCombo_{};
  QLineEdit *spotEdit_{};
  QLineEdit *volEdit_{};
  QLineEdit *rateEdit_{};
  QLineEdit *notionalEdit_{};
  QLineEdit *couponEdit_{};
  QLineEdit *autocallEdit_{};
  QLineEdit *couponBarrierEdit_{};
  QLineEdit *callBarriersEdit_{};
  QLineEdit *protectionEdit_{};
  QLineEdit *timesEdit_{};
  QLineEdit *pathsEdit_{};
  QLineEdit *seedEdit_{};
  QLineEdit *spreadEdit_{};
  QLineEdit *airbagEdit_{};
  QLineEdit *cliquetParticipationEdit_{};
  QLineEdit *cliquetCapEdit_{};
  QLineEdit *hestonV0Edit_{};
  QLineEdit *hestonKappaEdit_{};
  QLineEdit *hestonThetaEdit_{};
  QLineEdit *hestonXiEdit_{};
  QLineEdit *hestonRhoEdit_{};

  QLabel *priceLabel_{};
  QLabel *stdErrorLabel_{};
  QLabel *deltaLabel_{};
  QLabel *vegaLabel_{};
  QLabel *bidLabel_{};
  QLabel *askLabel_{};
  QLabel *chartLabel_{};
  QChartView *chartView_{};

  QGroupBox *productGroup_{};
  QGroupBox *cliquetGroup_{};
  QGroupBox *modelGroup_{};
  QFormLayout *productLayout_{};
  QFormLayout *cliquetLayout_{};
  QFormLayout *modelLayout_{};
  QWidget *autocallLabel_{};
  QWidget *cliquetLabel_{};
  QWidget *couponBarrierLabel_{};
  QWidget *callBarriersLabel_{};
  QWidget *airbagLabel_{};
  QWidget *cliquetParticipationLabel_{};
  QWidget *cliquetCapLabel_{};
  QWidget *sigmaLabel_{};
  QWidget *hestonV0Label_{};
  QWidget *hestonKappaLabel_{};
  QWidget *hestonThetaLabel_{};
  QWidget *hestonXiLabel_{};
  QWidget *hestonRhoLabel_{};

  void updateProductSpecificFields();
};

PricerWindow::PricerWindow(QWidget *parent) : QWidget(parent) {
  setWindowTitle("Structured Products");
  setMinimumSize(900, 700);
  resize(1650, 1000);
  defaults_.callBarriers = {4200.0, 4100.0, 4000.0, 3900.0};
  defaults_.couponBarrier = 3900.0;

  // Build the two-column layout: left inputs (scrollable), right chart/results.
  auto *mainLayout = new QHBoxLayout();
  mainLayout->setContentsMargins(16, 16, 16, 16);
  mainLayout->setSpacing(20);
  setLayout(mainLayout);
  inputContainer_ = new QWidget();
  auto *leftLayout = new QVBoxLayout(inputContainer_);
  leftLayout->setContentsMargins(0, 0, 0, 0);
  leftLayout->setSpacing(16);
  // General inputs: family/type selection, market data, schedule, MC controls.
  auto *generalGroup = new QGroupBox("General Inputs");
  auto *generalForm = new QFormLayout(generalGroup);
  generalForm->setSpacing(10);

  familyCombo_ = new QComboBox();
  familyCombo_->addItem("Autocall");
  familyCombo_->addItem("Cliquet");
  autocallCombo_ = new QComboBox();
  autocallCombo_->addItem("Simple Autocall");
  autocallCombo_->addItem("Phoenix Autocall");
  autocallCombo_->addItem("Memory Phoenix");
  autocallCombo_->addItem("Step-Down Autocall");
  autocallCombo_->addItem("Airbag Autocall");
  cliquetCombo_ = new QComboBox();
  cliquetCombo_->addItem("Cliquet Max Return");
  cliquetCombo_->addItem("Cliquet Capped Coupons");
  modelCombo_ = new QComboBox();
  modelCombo_->addItem("Black-Scholes");
  modelCombo_->addItem("Heston");
  spotEdit_ = new QLineEdit(doubleToQString(defaults_.spot));
  volEdit_ = new QLineEdit(doubleToQString(defaults_.sigma));
  rateEdit_ = new QLineEdit(doubleToQString(defaults_.rate));
  notionalEdit_ = new QLineEdit(doubleToQString(defaults_.notional));
  couponEdit_ = new QLineEdit(doubleToQString(defaults_.coupon));
  autocallEdit_ = new QLineEdit(doubleToQString(defaults_.autocallBarrier));
  couponBarrierEdit_ = new QLineEdit(doubleToQString(defaults_.couponBarrier));
  callBarriersEdit_ = new QLineEdit(
      QString::fromStdString(vectorToString(defaultCallBarrierList())));
  protectionEdit_ = new QLineEdit(doubleToQString(defaults_.protectionBarrier));
  timesEdit_ = new QLineEdit(
      QString::fromStdString(vectorToString(defaults_.observationTimes)));
  pathsEdit_ = new QLineEdit(sizeToQString(defaults_.paths));
  seedEdit_ = new QLineEdit(uintToQString(defaults_.seed));
  spreadEdit_ = new QLineEdit(doubleToQString(defaults_.spreadFraction));

  generalForm->addRow("Product family", familyCombo_);
  generalForm->addRow("Autocall type", autocallCombo_);
  autocallLabel_ = generalForm->labelForField(autocallCombo_);
  generalForm->addRow("Cliquet type", cliquetCombo_);
  cliquetLabel_ = generalForm->labelForField(cliquetCombo_);
  generalForm->addRow("Model", modelCombo_);
  generalForm->addRow("Spot", spotEdit_);
  generalForm->addRow("Rate", rateEdit_);
  generalForm->addRow("Notional", notionalEdit_);
  generalForm->addRow("Coupon", couponEdit_);
  generalForm->addRow("Autocall barrier", autocallEdit_);
  generalForm->addRow("Protection barrier", protectionEdit_);
  generalForm->addRow("Observation times", timesEdit_);
  generalForm->addRow("MC paths", pathsEdit_);
  generalForm->addRow("Seed", seedEdit_);
  generalForm->addRow("Spread (fraction)", spreadEdit_);
  leftLayout->addWidget(generalGroup);

  // Autocall-specific parameters (shown when relevant).
  productGroup_ = new QGroupBox("Product Parameters");
  productLayout_ = new QFormLayout(productGroup_);
  productLayout_->setSpacing(10);
  productLayout_->addRow("Coupon barrier", couponBarrierEdit_);
  couponBarrierLabel_ = productLayout_->labelForField(couponBarrierEdit_);
  productLayout_->addRow("Call barriers", callBarriersEdit_);
  callBarriersLabel_ = productLayout_->labelForField(callBarriersEdit_);
  airbagEdit_ = new QLineEdit(doubleToQString(defaults_.airbagFloor));
  productLayout_->addRow("Airbag floor", airbagEdit_);
  airbagLabel_ = productLayout_->labelForField(airbagEdit_);
  leftLayout->addWidget(productGroup_);

  // Cliquet-specific parameters (participation/cap for capped coupons).
  cliquetGroup_ = new QGroupBox("Cliquet Parameters");
  cliquetLayout_ = new QFormLayout(cliquetGroup_);
  cliquetLayout_->setSpacing(10);
  cliquetParticipationEdit_ =
      new QLineEdit(doubleToQString(defaults_.cliquetParticipation));
  cliquetLayout_->addRow("Participation", cliquetParticipationEdit_);
  cliquetParticipationLabel_ =
      cliquetLayout_->labelForField(cliquetParticipationEdit_);
  cliquetCapEdit_ = new QLineEdit(doubleToQString(defaults_.cliquetCap));
  cliquetLayout_->addRow("Coupon cap", cliquetCapEdit_);
  cliquetCapLabel_ = cliquetLayout_->labelForField(cliquetCapEdit_);
  leftLayout->addWidget(cliquetGroup_);

  // Model parameters (flat sigma or Heston set).
  modelGroup_ = new QGroupBox("Model Parameters");
  modelLayout_ = new QFormLayout(modelGroup_);
  modelLayout_->setSpacing(10);
  modelLayout_->addRow("Sigma", volEdit_);
  sigmaLabel_ = modelLayout_->labelForField(volEdit_);
  hestonV0Edit_ = new QLineEdit(doubleToQString(defaults_.hestonV0));
  hestonKappaEdit_ = new QLineEdit(doubleToQString(defaults_.hestonKappa));
  hestonThetaEdit_ = new QLineEdit(doubleToQString(defaults_.hestonTheta));
  hestonXiEdit_ = new QLineEdit(doubleToQString(defaults_.hestonXi));
  hestonRhoEdit_ = new QLineEdit(doubleToQString(defaults_.hestonRho));
  modelLayout_->addRow("Heston v0", hestonV0Edit_);
  hestonV0Label_ = modelLayout_->labelForField(hestonV0Edit_);
  modelLayout_->addRow("Heston kappa", hestonKappaEdit_);
  hestonKappaLabel_ = modelLayout_->labelForField(hestonKappaEdit_);
  modelLayout_->addRow("Heston theta", hestonThetaEdit_);
  hestonThetaLabel_ = modelLayout_->labelForField(hestonThetaEdit_);
  modelLayout_->addRow("Heston xi", hestonXiEdit_);
  hestonXiLabel_ = modelLayout_->labelForField(hestonXiEdit_);
  modelLayout_->addRow("Heston rho", hestonRhoEdit_);
  hestonRhoLabel_ = modelLayout_->labelForField(hestonRhoEdit_);
  leftLayout->addWidget(modelGroup_);

  // Action button to trigger pricing.
  auto *button = new QPushButton("Price");
  leftLayout->addWidget(button);

  // Display area for pricing outputs.
  auto *resultsLayout = new QFormLayout();
  resultsLayout->setSpacing(8);
  priceLabel_ = new QLabel("-");
  stdErrorLabel_ = new QLabel("-");
  deltaLabel_ = new QLabel("-");
  vegaLabel_ = new QLabel("-");
  bidLabel_ = new QLabel("-");
  askLabel_ = new QLabel("-");

  resultsLayout->addRow("Price", priceLabel_);
  resultsLayout->addRow("Std error", stdErrorLabel_);
  resultsLayout->addRow("Delta", deltaLabel_);
  resultsLayout->addRow("Vega", vegaLabel_);
  resultsLayout->addRow("Bid", bidLabel_);
  resultsLayout->addRow("Ask", askLabel_);

  leftLayout->addLayout(resultsLayout);
  leftLayout->addStretch();

  // Scroll wrapper so tall configurations (e.g., Heston + cliquet params) stay
  // usable.
  inputScroll_ = new QScrollArea();
  inputScroll_->setWidgetResizable(true);
  inputScroll_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  inputScroll_->setFrameShape(QFrame::NoFrame);
  inputScroll_->setWidget(inputContainer_);
  mainLayout->addWidget(inputScroll_, 2);

  // Right side: payoff chart + legend (non-scrolling).
  auto *rightContainer = new QWidget();
  auto *rightLayout = new QVBoxLayout(rightContainer);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  rightLayout->setSpacing(10);
  chartLabel_ = new QLabel("Terminal payoff (linear scenario)");
  chartView_ = new QChartView(new QChart());
  chartView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  chartView_->setRenderHint(QPainter::Antialiasing);
  chartView_->chart()->legend()->setVisible(false);
  rightLayout->addWidget(chartLabel_);
  rightLayout->addWidget(chartView_, 1);
  mainLayout->addWidget(rightContainer, 3);
  mainLayout->setStretch(0, 2);
  mainLayout->setStretch(1, 3);

  connect(button, &QPushButton::clicked, this, &PricerWindow::handlePrice);
  connect(familyCombo_, &QComboBox::currentIndexChanged, this,
          &PricerWindow::updateProductSpecificFields);
  connect(autocallCombo_, &QComboBox::currentIndexChanged, this,
          &PricerWindow::updateProductSpecificFields);
  connect(cliquetCombo_, &QComboBox::currentIndexChanged, this,
          &PricerWindow::updateProductSpecificFields);
  connect(modelCombo_, &QComboBox::currentIndexChanged, this,
          &PricerWindow::updateProductSpecificFields);
  connectInputs();
  updateProductSpecificFields();
  updatePayoffChart();
  loadSettings();
}

QString PricerWindow::doubleToQString(double value) {
  return QString::number(value, 'g', 12);
}

QString PricerWindow::sizeToQString(std::size_t value) {
  return QString::number(static_cast<qulonglong>(value));
}

QString PricerWindow::uintToQString(unsigned int value) {
  return QString::number(static_cast<qulonglong>(value));
}

double PricerWindow::readDouble(QLineEdit *edit, double fallback) const {
  const QString text = edit->text().trimmed();
  if (text.isEmpty()) {
    return fallback;
  }
  bool ok = false;
  const double value = text.toDouble(&ok);
  if (!ok) {
    throw std::runtime_error("Invalid number: " + text.toStdString());
  }
  return value;
}

std::size_t PricerWindow::readSizeT(QLineEdit *edit,
                                    std::size_t fallback) const {
  const QString text = edit->text().trimmed();
  if (text.isEmpty()) {
    return fallback;
  }
  bool ok = false;
  const qulonglong value = text.toULongLong(&ok);
  if (!ok) {
    throw std::runtime_error("Invalid integer: " + text.toStdString());
  }
  return static_cast<std::size_t>(value);
}

unsigned int PricerWindow::readUInt(QLineEdit *edit,
                                    unsigned int fallback) const {
  const QString text = edit->text().trimmed();
  if (text.isEmpty()) {
    return fallback;
  }
  bool ok = false;
  const qulonglong value = text.toULongLong(&ok);
  if (!ok) {
    throw std::runtime_error("Invalid integer: " + text.toStdString());
  }
  return static_cast<unsigned int>(value);
}

// Pulls all UI inputs into a PricingInputs structure.
PricingInputs PricerWindow::gatherInputs() const {
  PricingInputs inputs = defaults_;
  inputs.productFamily = familyCombo_->currentIndex() == 1
                             ? ProductFamily::Cliquet
                             : ProductFamily::Autocall;
  if (inputs.productFamily == ProductFamily::Autocall) {
    switch (autocallCombo_->currentIndex()) {
    case 1:
      inputs.autocallType = AutocallType::Phoenix;
      break;
    case 2:
      inputs.autocallType = AutocallType::MemoryPhoenix;
      break;
    case 3:
      inputs.autocallType = AutocallType::StepDown;
      break;
    case 4:
      inputs.autocallType = AutocallType::Airbag;
      break;
    default:
      inputs.autocallType = AutocallType::Simple;
      break;
    }
  } else {
    inputs.cliquetType = cliquetCombo_->currentIndex() == 1
                             ? CliquetType::CappedCoupons
                             : CliquetType::MaxReturn;
  }
  inputs.modelType = modelCombo_->currentIndex() == 1 ? ModelType::Heston
                                                      : ModelType::BlackScholes;
  inputs.spot = readDouble(spotEdit_, defaults_.spot);
  inputs.sigma = readDouble(volEdit_, defaults_.sigma);
  inputs.rate = readDouble(rateEdit_, defaults_.rate);
  inputs.notional = readDouble(notionalEdit_, defaults_.notional);
  inputs.coupon = readDouble(couponEdit_, defaults_.coupon);
  inputs.autocallBarrier = readDouble(autocallEdit_, defaults_.autocallBarrier);
  inputs.couponBarrier =
      readDouble(couponBarrierEdit_, defaults_.couponBarrier);
  inputs.protectionBarrier =
      readDouble(protectionEdit_, defaults_.protectionBarrier);
  if (inputs.productFamily == ProductFamily::Autocall &&
      inputs.autocallType == AutocallType::StepDown) {
    inputs.callBarriers = parseTimesList(
        callBarriersEdit_->text().trimmed().toStdString(), inputs.callBarriers);
  }
  if (inputs.productFamily == ProductFamily::Autocall &&
      inputs.autocallType == AutocallType::Airbag) {
    inputs.airbagFloor = readDouble(airbagEdit_, defaults_.airbagFloor);
  }
  if (inputs.productFamily == ProductFamily::Cliquet) {
    inputs.cliquetParticipation =
        readDouble(cliquetParticipationEdit_, defaults_.cliquetParticipation);
    inputs.cliquetCap = readDouble(cliquetCapEdit_, defaults_.cliquetCap);
  }
  if (inputs.modelType == ModelType::Heston) {
    inputs.hestonV0 = readDouble(hestonV0Edit_, defaults_.hestonV0);
    inputs.hestonKappa = readDouble(hestonKappaEdit_, defaults_.hestonKappa);
    inputs.hestonTheta = readDouble(hestonThetaEdit_, defaults_.hestonTheta);
    inputs.hestonXi = readDouble(hestonXiEdit_, defaults_.hestonXi);
    inputs.hestonRho = readDouble(hestonRhoEdit_, defaults_.hestonRho);
  }

  inputs.observationTimes = parseTimesList(
      timesEdit_->text().trimmed().toStdString(), defaults_.observationTimes);
  inputs.paths = readSizeT(pathsEdit_, defaults_.paths);
  inputs.seed = readUInt(seedEdit_, defaults_.seed);
  inputs.spreadFraction = readDouble(spreadEdit_, defaults_.spreadFraction);
  return inputs;
}

void PricerWindow::handlePrice() {
  try {
    PricingInputs inputs = gatherInputs();
    const PricingResults results = priceAutocall(inputs);
    updateResults(results);
    updatePayoffChart();
  } catch (const std::exception &ex) {
    showError(QString::fromStdString(ex.what()));
  }
}

void PricerWindow::updateResults(const PricingResults &results) {
  priceLabel_->setText(QString::number(results.price, 'f', 4));
  stdErrorLabel_->setText(QString::number(results.stdError, 'f', 4));
  deltaLabel_->setText(QString::number(results.delta, 'f', 4));
  vegaLabel_->setText(QString::number(results.vega, 'f', 4));
  bidLabel_->setText(QString::number(results.bid, 'f', 4));
  askLabel_->setText(QString::number(results.ask, 'f', 4));
}

void PricerWindow::showError(const QString &message) {
  QMessageBox::warning(this, "Pricing failed", message);
}

// Show/hide groups based on selected family/type and refresh layout/scroll
// area.
void PricerWindow::updateProductSpecificFields() {
  const bool isCliquetFamily = familyCombo_->currentIndex() == 1;
  const bool isAutocallFamily = !isCliquetFamily;
  autocallLabel_->setVisible(isAutocallFamily);
  autocallCombo_->setVisible(isAutocallFamily);
  cliquetLabel_->setVisible(isCliquetFamily);
  cliquetCombo_->setVisible(isCliquetFamily);

  const int autocallIndex = autocallCombo_->currentIndex();
  const bool isPhoenixLike =
      isAutocallFamily && (autocallIndex == 1 || autocallIndex == 2);
  const bool isStepDown = isAutocallFamily && autocallIndex == 3;
  const bool isAirbag = isAutocallFamily && autocallIndex == 4;
  couponBarrierLabel_->setVisible(isPhoenixLike);
  couponBarrierEdit_->setVisible(isPhoenixLike);
  callBarriersLabel_->setVisible(isStepDown);
  callBarriersEdit_->setVisible(isStepDown);
  airbagLabel_->setVisible(isAirbag);
  airbagEdit_->setVisible(isAirbag);
  const bool showAutocallGroup =
      isAutocallFamily && (isPhoenixLike || isStepDown || isAirbag);
  productGroup_->setVisible(showAutocallGroup);

  const int cliquetIndex = cliquetCombo_->currentIndex();
  const bool showCliquetParams =
      isCliquetFamily && cliquetIndex == 1; // capped coupons needs params
  cliquetGroup_->setVisible(showCliquetParams);
  cliquetParticipationLabel_->setVisible(showCliquetParams);
  cliquetParticipationEdit_->setVisible(showCliquetParams);
  cliquetCapLabel_->setVisible(showCliquetParams);
  cliquetCapEdit_->setVisible(showCliquetParams);

  const bool isHeston = modelCombo_->currentIndex() == 1;
  sigmaLabel_->setVisible(!isHeston);
  volEdit_->setVisible(!isHeston);
  hestonV0Label_->setVisible(isHeston);
  hestonV0Edit_->setVisible(isHeston);
  hestonKappaLabel_->setVisible(isHeston);
  hestonKappaEdit_->setVisible(isHeston);
  hestonThetaLabel_->setVisible(isHeston);
  hestonThetaEdit_->setVisible(isHeston);
  hestonXiLabel_->setVisible(isHeston);
  hestonXiEdit_->setVisible(isHeston);
  hestonRhoLabel_->setVisible(isHeston);
  hestonRhoEdit_->setVisible(isHeston);
  modelGroup_->setVisible(true);

  if (inputContainer_) {
    inputContainer_->adjustSize();
    inputContainer_->updateGeometry();
  }
  if (inputScroll_) {
    if (auto *widget = inputScroll_->widget()) {
      widget->adjustSize();
      widget->updateGeometry();
    }
    inputScroll_->updateGeometry();
  }
  if (auto *lay = layout()) {
    lay->invalidate();
    lay->activate();
  }
  adjustSize();
  updatePayoffChart();
}

std::vector<double> PricerWindow::defaultCallBarrierList() const {
  if (!defaults_.callBarriers.empty()) {
    return defaults_.callBarriers;
  }
  return std::vector<double>(defaults_.observationTimes.size(),
                             defaults_.autocallBarrier);
}

void PricerWindow::connectInputField(QLineEdit *edit) {
  connect(edit, &QLineEdit::editingFinished, this,
          &PricerWindow::updatePayoffChart);
}

void PricerWindow::connectInputs() {
  connectInputField(spotEdit_);
  connectInputField(volEdit_);
  connectInputField(rateEdit_);
  connectInputField(notionalEdit_);
  connectInputField(couponEdit_);
  connectInputField(autocallEdit_);
  connectInputField(couponBarrierEdit_);
  connectInputField(callBarriersEdit_);
  connectInputField(protectionEdit_);
  connectInputField(timesEdit_);
  connectInputField(pathsEdit_);
  connectInputField(seedEdit_);
  connectInputField(spreadEdit_);
  connectInputField(airbagEdit_);
  connectInputField(cliquetParticipationEdit_);
  connectInputField(cliquetCapEdit_);
  connectInputField(hestonV0Edit_);
  connectInputField(hestonKappaEdit_);
  connectInputField(hestonThetaEdit_);
  connectInputField(hestonXiEdit_);
  connectInputField(hestonRhoEdit_);
}

void PricerWindow::closeEvent(QCloseEvent *event) {
  saveSettings();
  QWidget::closeEvent(event);
}

// Persist/restore window geometry for convenience.
void PricerWindow::loadSettings() {
  QSettings settings("ENSAE", "AutocallPricer");
  const QByteArray geometry = settings.value("geometry").toByteArray();
  if (!geometry.isEmpty()) {
    restoreGeometry(geometry);
  }
}

void PricerWindow::saveSettings() const {
  QSettings settings("ENSAE", "AutocallPricer");
  settings.setValue("geometry", saveGeometry());
}

// Build the correct StructuredProduct for the chosen family/type.
std::unique_ptr<StructuredProduct>
PricerWindow::createProduct(const PricingInputs &inputs) const {
  if (inputs.productFamily == ProductFamily::Autocall) {
    switch (inputs.autocallType) {
    case AutocallType::Simple:
      return std::make_unique<SimpleAutocall>(
          inputs.underlying, inputs.observationTimes, inputs.spot,
          inputs.notional, inputs.coupon, inputs.autocallBarrier,
          inputs.protectionBarrier);
    case AutocallType::Phoenix:
      return std::make_unique<PhoenixAutocall>(
          inputs.underlying, inputs.observationTimes, inputs.spot,
          inputs.notional, inputs.coupon, inputs.autocallBarrier,
          inputs.protectionBarrier, inputs.couponBarrier);
    case AutocallType::MemoryPhoenix:
      return std::make_unique<MemoryPhoenixAutocall>(
          inputs.underlying, inputs.observationTimes, inputs.spot,
          inputs.notional, inputs.coupon, inputs.autocallBarrier,
          inputs.protectionBarrier, inputs.couponBarrier);
    case AutocallType::StepDown: {
      std::vector<double> schedule = inputs.callBarriers;
      if (schedule.empty()) {
        schedule.assign(inputs.observationTimes.size(), inputs.autocallBarrier);
      }
      return std::make_unique<StepDownAutocall>(
          inputs.underlying, inputs.observationTimes, inputs.spot,
          inputs.notional, inputs.coupon, schedule, inputs.protectionBarrier);
    }
    case AutocallType::Airbag:
      return std::make_unique<AirbagAutocall>(
          inputs.underlying, inputs.observationTimes, inputs.spot,
          inputs.notional, inputs.coupon, inputs.autocallBarrier,
          inputs.protectionBarrier, inputs.airbagFloor);
    }
  } else {
    switch (inputs.cliquetType) {
    case CliquetType::MaxReturn:
      return std::make_unique<CliquetMaxReturn>(inputs.underlying,
                                                inputs.observationTimes,
                                                inputs.spot, inputs.notional);
    case CliquetType::CappedCoupons:
      return std::make_unique<CliquetCappedCoupons>(
          inputs.underlying, inputs.observationTimes, inputs.spot,
          inputs.notional, inputs.cliquetParticipation, inputs.cliquetCap);
    }
  }
  return nullptr;
}

// Refresh the payoff chart.
// Uses a simulated linear path (S0 -> ST) to estimate payoff for visualization.
void PricerWindow::updatePayoffChart() {
  QChart *chart = chartView_->chart();
  try {
    const PricingInputs inputs = gatherInputs();
    auto product = createProduct(inputs);
    if (!product) {
      chart->removeAllSeries();
      return;
    }
    if (inputs.productFamily == ProductFamily::Cliquet) {
      chartLabel_->setText(
          "Cliquet payoff shown for a reference linear path (path-dependent)");
    } else {
      chartLabel_->setText("Terminal payoff (linear scenario)");
    }

    const double spot = std::max(inputs.spot, 1.0);
    const double maxSpot = spot * 1.5;
    constexpr int samples = 200;
    auto *payoffSeries = new QLineSeries();
    payoffSeries->setName("Redemption");

    double minY = std::numeric_limits<double>::max();
    double maxYValue = std::numeric_limits<double>::lowest();

    const auto &times = product->observationTimes();
    const double finalTime = times.empty() ? 1.0 : times.back();

    for (int i = 0; i < samples; ++i) {
      const double st =
          maxSpot * static_cast<double>(i) / static_cast<double>(samples - 1);

      // Build a synthetic linear path from S0 to ST
      std::vector<double> path;
      path.reserve(times.size());
      for (double t : times) {
        // S(t) = S0 + (ST - S0) * (t / T)
        double val = inputs.spot + (st - inputs.spot) * (t / finalTime);
        if (val < 0.0)
          val = 0.0;
        path.push_back(val);
      }

      // Calculate total payoff (sum of all flows)
      // Use 0.0 rate to get the raw sum of flows for the chart
      double totalPayoff = product->discountedPayoff(path, 0.0);

      payoffSeries->append(st, totalPayoff);
      minY = std::min(minY, totalPayoff);
      maxYValue = std::max(maxYValue, totalPayoff);
    }

    if (minY == maxYValue) {
      minY *= 0.9;
      maxYValue *= 1.1;
    }

    chart->removeAllSeries();
    const auto axes = chart->axes();
    for (QAbstractAxis *axis : axes) {
      chart->removeAxis(axis);
      delete axis;
    }
    chart->addSeries(payoffSeries);

    auto *axisX = new QValueAxis();
    axisX->setRange(0.0, maxSpot);
    axisX->setTitleText("Terminal spot S_T");
    auto *axisY = new QValueAxis();
    axisY->setRange(minY, maxYValue);
    axisY->setTitleText("Total Payoff (Redemption + Coupons)");
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    payoffSeries->attachAxis(axisX);
    payoffSeries->attachAxis(axisY);

    auto addMarker = [&](double x, const QString &label, const QColor &color) {
      if (x <= 0.0 || x > axisX->max()) {
        return;
      }
      auto *marker = new QLineSeries();
      marker->setName(label);
      QPen pen(color);
      pen.setStyle(Qt::DashLine);
      pen.setWidthF(1.0);
      marker->setPen(pen);
      marker->append(x, minY);
      marker->append(x, maxYValue);
      chart->addSeries(marker);
      marker->attachAxis(axisX);
      marker->attachAxis(axisY);
    };

    if (inputs.productFamily == ProductFamily::Autocall) {
      addMarker(inputs.protectionBarrier, "Protection barrier",
                QColor("#c62828"));
      if (inputs.autocallType == AutocallType::StepDown) {
        std::vector<double> schedule = inputs.callBarriers;
        if (schedule.empty()) {
          schedule.assign(inputs.observationTimes.size(),
                          inputs.autocallBarrier);
        }
        for (double barrier : schedule) {
          addMarker(barrier, "Call barrier", QColor("#1565c0"));
        }
      } else {
        addMarker(inputs.autocallBarrier, "Call barrier", QColor("#1565c0"));
      }

      if (inputs.autocallType == AutocallType::Phoenix ||
          inputs.autocallType == AutocallType::MemoryPhoenix) {
        addMarker(inputs.couponBarrier, "Coupon barrier", QColor("#2e7d32"));
      }
    }

    chart->legend()->setVisible(chart->series().size() > 1);
  } catch (...) {
    chart->removeAllSeries();
    const auto axes = chart->axes();
    for (QAbstractAxis *axis : axes) {
      chart->removeAxis(axis);
      delete axis;
    }
  }
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  PricerWindow window;
  window.show();
  return app.exec();
}

#include "main.moc"