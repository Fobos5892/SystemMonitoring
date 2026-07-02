#include "Domain/filterqueryspec.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Application/telemetryfacade.h"
#include "ViewModels/filterviewmodel.h"
#include "ViewModels/statisticsviewmodel.h"
#include "ViewModels/telemetrytablemodel.h"
#include "ViewModels/telemetryviewmodel.h"
#include <QDateTime>
#include <QHeaderView>
#include <QIcon>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QStyle>
#include <QtGlobal>

namespace {

constexpr int LOADING_ANIMATION_INTERVAL_MS = 80;

const QStringList &loadingSpinnerFrames()
{
    static const QStringList frames = {
        QStringLiteral("⠋"),
        QStringLiteral("⠙"),
        QStringLiteral("⠹"),
        QStringLiteral("⠸"),
        QStringLiteral("⠼"),
        QStringLiteral("⠴"),
        QStringLiteral("⠦"),
        QStringLiteral("⠧"),
        QStringLiteral("⠇"),
        QStringLiteral("⠏"),
    };
    return frames;
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , facade(new TelemetryFacade())
    , loadingAnimationTimer(new QTimer(this))
    , scrollIdleTimer(new QTimer(this))
{
    ui->setupUi(this);

    TelemetryTableModel *const tableModel = facade->tableModel();
    TelemetryViewModel *const viewModel = facade->telemetryViewModel();

    ui->SystemTableView->setModel(tableModel);
    configureTableColumns();
    ui->SystemTableView->horizontalHeader()->setStretchLastSection(true);

    facade->start();

    ui->SystemTableView->sortByColumn(static_cast<int>(Telemetry::Column::Timestamp),
                                      Qt::AscendingOrder);

    connect(ui->SystemTableView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int) {
        onScrollActivity();
        updateFollowMode();
    });
    connect(ui->SystemTableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, [this](int, Qt::SortOrder) { updateFollowMode(); });

    scrollIdleTimer->setSingleShot(true);
    scrollIdleTimer->setInterval(SCROLL_IDLE_MS);
    connect(scrollIdleTimer.data(), &QTimer::timeout, this, &MainWindow::onScrollIdleTimeout);

    connect(viewModel, &TelemetryViewModel::liveDataInserted, this, [this, viewModel]() {
        if (!isGenerating) {
            return;
        }
        if (!viewModel->isFollowingLiveTail()) {
            return;
        }
        const Qt::SortOrder order = ui->SystemTableView->horizontalHeader()->sortIndicatorOrder();
        if (order == Qt::AscendingOrder) {
            ui->SystemTableView->scrollToBottom();
        } else {
            ui->SystemTableView->scrollToTop();
        }
    });

    connect(viewModel, &TelemetryViewModel::loadingStarted, this, &MainWindow::acquireUiLock);
    connect(viewModel, &TelemetryViewModel::loadingFinished, this, [this]() {
        releaseUiLock();
        updateFollowMode();
        updateViewportZone();
    });

    connect(viewModel, &TelemetryViewModel::mergeStarted, this, [this]() {
        setTableScrollLocked(true);
    });
    connect(viewModel, &TelemetryViewModel::mergeFinished, this, [this]() {
        setTableScrollLocked(false);
    });

    connect(facade.data(), &TelemetryFacade::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);

    loadingAnimationTimer->setInterval(LOADING_ANIMATION_INTERVAL_MS);
    connect(loadingAnimationTimer.data(), &QTimer::timeout, this, [this]() {
        const QStringList &frames = loadingSpinnerFrames();
        loadingAnimationFrame = (loadingAnimationFrame + 1) % frames.size();
        ui->spinnerAnimationLabel->setText(frames.at(loadingAnimationFrame));
    });

    ui->tableStackedWidget->setCurrentWidget(ui->tablePage);

    configureFilterControls();
    resetFilterControlsToDefault();

    ui->resetFilterButton->setIcon(QIcon(QStringLiteral(":/assets/filter-reset.svg")));

    bindStatisticsLabels();

    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::onConnectButtonClicked);
    connect(ui->clearDatabaseButton, &QPushButton::clicked,
            this, &MainWindow::onClearDatabaseClicked);
    connect(ui->applyFilterButton, &QPushButton::clicked,
            this, &MainWindow::onApplyFilterClicked);
    connect(ui->resetFilterButton, &QToolButton::clicked,
            this, &MainWindow::onResetFilterClicked);
    connect(ui->filterFieldComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterFieldChanged);
    connect(ui->filterDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double) { normalizeNumericFilterInputs(); });
    connect(ui->filterToleranceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double) { normalizeNumericFilterInputs(); });
    connect(ui->filterDoubleSpinBox, &QDoubleSpinBox::editingFinished,
            this, &MainWindow::normalizeNumericFilterInputs);
    connect(ui->filterToleranceSpinBox, &QDoubleSpinBox::editingFinished,
            this, &MainWindow::normalizeNumericFilterInputs);

    updateFollowMode();
    updateViewportZone();
    facade->telemetryViewModel()->setScrollIdle(true);
}

MainWindow::~MainWindow() = default;

TelemetryTableModel* MainWindow::getTableModel() const
{
    return facade->tableModel();
}

void MainWindow::bindStatisticsLabels()
{
    StatisticsViewModel *const statsVm = facade->statisticsViewModel();
    connect(statsVm, &StatisticsViewModel::labelsChanged,
            this, &MainWindow::onStatisticsLabelsChanged);
    onStatisticsLabelsChanged();
}

void MainWindow::onStatisticsLabelsChanged()
{
    const StatisticsViewModel *const statsVm = facade->statisticsViewModel();
    ui->statsConnectedLabel->setText(statsVm->connectedLabel());
    ui->statsAverageLabel->setText(statsVm->averageLabel());
    ui->statsMinLabel->setText(statsVm->minimumLabel());
    ui->statsMaxLabel->setText(statsVm->maximumLabel());
}

void MainWindow::updateFollowMode()
{
    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    const QScrollBar *scrollBar = ui->SystemTableView->verticalScrollBar();
    const bool atTop = scrollBar->value() <= scrollBar->minimum();
    const bool atBottom = scrollBar->value() >= scrollBar->maximum();
    const Qt::SortOrder order = ui->SystemTableView->horizontalHeader()->sortIndicatorOrder();
    viewModel->setFollowLiveTail(order == Qt::AscendingOrder ? atBottom : atTop);
}

void MainWindow::updateViewportZone()
{
    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    const QScrollBar *scrollBar = ui->SystemTableView->verticalScrollBar();
    const bool atTop = scrollBar->value() <= scrollBar->minimum();
    const bool atBottom = scrollBar->value() >= scrollBar->maximum();

    if (atBottom) {
        viewModel->setViewportZone(Telemetry::ViewportZone::BottomEdge);
        return;
    }
    if (atTop) {
        viewModel->setViewportZone(Telemetry::ViewportZone::TopEdge);
        return;
    }
    viewModel->setViewportZone(Telemetry::ViewportZone::Middle);
}

void MainWindow::onScrollActivity()
{
    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    viewModel->setScrollIdle(false);
    scrollIdleTimer->start();
}

void MainWindow::onScrollIdleTimeout()
{
    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    viewModel->setScrollIdle(true);
    updateViewportZone();
}

void MainWindow::setTableScrollLocked(bool locked)
{
    tableScrollLocked = locked;
    QScrollBar *scrollBar = ui->SystemTableView->verticalScrollBar();
    scrollBar->setEnabled(!locked && isUiInteractive());
    ui->SystemTableView->setVerticalScrollBarPolicy(
        locked ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
}

void MainWindow::onConnectButtonClicked()
{
    if (!isUiInteractive()) {
        return;
    }

    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    if (isGenerating) {
        acquireUiLock();
        viewModel->setLiveUpdatesEnabled(false);
        viewModel->setFollowLiveTail(false);
        facade->stopGeneration();
        return;
    }

    acquireUiLock();
    viewModel->setLiveUpdatesEnabled(true);
    viewModel->setFilterMode(false);
    viewModel->setFollowLiveTail(true);
    facade->startGeneration();
}

void MainWindow::onConnectionStatusChanged(Telemetry::ConnectionStatus status)
{
    switch (status) {
    case Telemetry::ConnectionStatus::Started:
        setGenerationRunning(true);
        releaseUiLock();
        updateFollowMode();
        break;
    case Telemetry::ConnectionStatus::Stopped:
        setGenerationRunning(false);
        releaseUiLock();
        break;
    }
}

void MainWindow::onClearDatabaseClicked()
{
    if (!isUiInteractive()) {
        return;
    }

    facade->clearDatabase();
}

void MainWindow::syncFilterViewModelFromUi()
{
    FilterViewModel *const filterVm = facade->filterViewModel();
    const QDateTime fromDateTime =
        FilterViewModel::combineLocalDateTime(ui->filterDateFrom->date(), ui->filterTimeFrom->time());
    const QDateTime toDateTime = FilterViewModel::combineLocalDateTime(
        ui->filterDateTo->date(), ui->filterTimeTo->time(), true);
    filterVm->setField(static_cast<FilterViewModel::Field>(ui->filterFieldComboBox->currentIndex()));
    filterVm->setSensorId(ui->filterSpinBox->value());
    filterVm->setValueFilter(
        ui->filterDoubleSpinBox->value(),
        ui->filterToleranceSpinBox->value(),
        static_cast<FilterViewModel::ValueOperation>(ui->filterValueOperationComboBox->currentIndex()));
    filterVm->setTimestampRange(fromDateTime, toDateTime);
}

void MainWindow::resetFilterControlsToDefault()
{
    ui->groupBoxFilter->setEnabled(false);

    ui->filterFieldComboBox->setCurrentIndex(0);
    ui->filterSpinBox->setValue(FilterQuerySpec::DEFAULT_SENSOR_ID);
    ui->filterValueOperationComboBox->setCurrentIndex(0);
    ui->filterDoubleSpinBox->setValue(FilterQuerySpec::DEFAULT_VALUE);
    ui->filterToleranceSpinBox->setValue(FilterQuerySpec::DEFAULT_TOLERANCE);

    const QDateTime now = QDateTime::currentDateTime();
    ui->filterDateFrom->setDate(now.date());
    ui->filterTimeFrom->setTime(FilterViewModel::DAY_START_TIME);
    ui->filterDateTo->setDate(now.date());
    ui->filterTimeTo->setTime(now.time());

    onFilterFieldChanged(ui->filterFieldComboBox->currentIndex());
    normalizeNumericFilterInputs();

    ui->groupBoxFilter->setEnabled(isUiInteractive());
}

void MainWindow::onApplyFilterClicked()
{
    if (!isUiInteractive()) {
        return;
    }

    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    syncFilterViewModelFromUi();
    viewModel->setLiveUpdatesEnabled(false);
    viewModel->setFilterMode(true);
    viewModel->setFollowLiveTail(false);
    const FilterQuerySpec filterSpec = facade->filterViewModel()->buildQuerySpec();
    const auto *const header = ui->SystemTableView->horizontalHeader();
    facade->applyFilter(filterSpec,
                        header->sortIndicatorSection(),
                        static_cast<int>(header->sortIndicatorOrder()),
                        computeFilterRequestLimit());
}

void MainWindow::onResetFilterClicked()
{
    if (!isUiInteractive()) {
        return;
    }

    resetFilterControlsToDefault();

    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    if (!viewModel->isFilterMode()) {
        return;
    }

    viewModel->setFilterMode(false);
    if (isGenerating) {
        viewModel->setLiveUpdatesEnabled(true);
    }

    const auto *const header = ui->SystemTableView->horizontalHeader();
    facade->resetFilter(header->sortIndicatorSection(),
                        static_cast<int>(header->sortIndicatorOrder()));
    updateFollowMode();
}

int MainWindow::computeFilterRequestLimit() const
{
    const int rowHeight = ui->SystemTableView->verticalHeader()->defaultSectionSize();
    if (rowHeight <= 0) {
        return Telemetry::WINDOW_SIZE;
    }

    const int visibleRows = ui->SystemTableView->viewport()->height() / rowHeight;
    const int bufferedRows = visibleRows + Telemetry::CHUNK_SIZE;
    return qBound(Telemetry::CHUNK_SIZE, bufferedRows, Telemetry::WINDOW_SIZE);
}

void MainWindow::onFilterFieldChanged(int index)
{
    ui->filterValueStack->setCurrentIndex(index);
}

void MainWindow::normalizeNumericFilterInputs()
{
    const double normalizedValue = FilterViewModel::normalizeValue(ui->filterDoubleSpinBox->value());
    const double normalizedTolerance =
        FilterViewModel::normalizeTolerance(ui->filterToleranceSpinBox->value());

    const QSignalBlocker valueBlocker(ui->filterDoubleSpinBox);
    const QSignalBlocker toleranceBlocker(ui->filterToleranceSpinBox);
    ui->filterDoubleSpinBox->setValue(normalizedValue);
    ui->filterToleranceSpinBox->setValue(normalizedTolerance);

    ui->filterToleranceSpinBox->setSingleStep(
        FilterViewModel::adaptiveToleranceStep(normalizedTolerance));
}

void MainWindow::setGenerationRunning(bool running)
{
    isGenerating = running;
    ui->connectButton->setText(running ? tr("Стоп") : tr("Подключиться"));
    ui->connectButton->setProperty("running", running);
    ui->connectButton->style()->unpolish(ui->connectButton);
    ui->connectButton->style()->polish(ui->connectButton);
}

void MainWindow::setLoadingOverlayVisible(bool visible)
{
    ui->tableStackedWidget->setCurrentWidget(visible ? ui->loadingPage : ui->tablePage);
}

bool MainWindow::isUiInteractive() const
{
    return uiLockDepth == 0;
}

void MainWindow::applyUiLockState()
{
    const bool enabled = isUiInteractive();
    ui->groupBoxFilter->setEnabled(enabled);
    ui->SystemTableView->setEnabled(enabled);
    ui->SystemTableView->horizontalHeader()->setSectionsClickable(enabled);
    ui->SystemTableView->verticalScrollBar()->setEnabled(enabled && !tableScrollLocked);
}

void MainWindow::acquireUiLock()
{
    ++uiLockDepth;
    if (uiLockDepth != 1) {
        return;
    }

    applyUiLockState();
    setLoadingOverlayVisible(true);
    startLoadingAnimation();
}

void MainWindow::releaseUiLock()
{
    if (uiLockDepth <= 0) {
        return;
    }

    --uiLockDepth;
    if (uiLockDepth != 0) {
        return;
    }

    applyUiLockState();
    setLoadingOverlayVisible(false);
    stopLoadingAnimation();
}

void MainWindow::startLoadingAnimation()
{
    loadingAnimationFrame = 0;
    ui->spinnerAnimationLabel->setText(loadingSpinnerFrames().first());
    loadingAnimationTimer->start();
}

void MainWindow::stopLoadingAnimation()
{
    loadingAnimationTimer->stop();
    ui->spinnerAnimationLabel->setText(QStringLiteral("⟳"));
}

void MainWindow::configureTableColumns()
{
    ui->SystemTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->SystemTableView->horizontalHeader()->setMinimumSectionSize(MIN_TABLE_COLUMN_WIDTH);
    ui->SystemTableView->setColumnWidth(static_cast<int>(Telemetry::Column::RecordId),
                                        RECORD_ID_COLUMN_WIDTH);
    ui->SystemTableView->setColumnWidth(static_cast<int>(Telemetry::Column::SensorId),
                                        SENSOR_ID_COLUMN_WIDTH);
    ui->SystemTableView->setColumnWidth(static_cast<int>(Telemetry::Column::Value),
                                        VALUE_COLUMN_WIDTH);
    ui->SystemTableView->setColumnWidth(static_cast<int>(Telemetry::Column::Timestamp),
                                        TIMESTAMP_COLUMN_WIDTH);
    ui->SystemTableView->verticalHeader()->setVisible(false);
}

void MainWindow::configureFilterControls()
{
    ui->filterSpinBox->setMinimum(FilterQuerySpec::DEFAULT_SENSOR_ID);
    ui->filterSpinBox->setMaximum(FilterLimits::MAX_SENSOR_ID);

    ui->filterDoubleSpinBox->setDecimals(FilterLimits::SENSOR_VALUE_DECIMAL_PLACES);
    ui->filterDoubleSpinBox->setMinimum(FilterLimits::MIN_SENSOR_VALUE_VOLTS);
    ui->filterDoubleSpinBox->setMaximum(FilterLimits::MAX_SENSOR_VALUE_VOLTS);
    ui->filterDoubleSpinBox->setSingleStep(FilterLimits::COARSE_TOLERANCE_STEP);

    ui->filterToleranceSpinBox->setDecimals(FilterLimits::TOLERANCE_DECIMAL_PLACES);
    ui->filterToleranceSpinBox->setMinimum(FilterLimits::MIN_TOLERANCE);
    ui->filterToleranceSpinBox->setMaximum(FilterLimits::MAX_TOLERANCE);
    ui->filterToleranceSpinBox->setSingleStep(FilterLimits::TOLERANCE_SPINBOX_SINGLE_STEP);
}
