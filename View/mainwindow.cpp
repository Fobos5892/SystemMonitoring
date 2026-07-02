#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Application/telemetryfacade.h"
#include "ViewModels/filterviewmodel.h"
#include "ViewModels/statisticsviewmodel.h"
#include "ViewModels/telemetrytablemodel.h"
#include "ViewModels/telemetryviewmodel.h"
#include <QDateTime>
#include <QHeaderView>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QStyle>
#include <QtGlobal>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , facade(new TelemetryFacade())
{
    ui->setupUi(this);

    TelemetryTableModel *const tableModel = facade->tableModel();
    TelemetryViewModel *const viewModel = facade->telemetryViewModel();

    ui->SystemTableView->setModel(tableModel);
    ui->SystemTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->SystemTableView->horizontalHeader()->setMinimumSectionSize(60);
    ui->SystemTableView->setColumnWidth(0, 90);
    ui->SystemTableView->setColumnWidth(1, 90);
    ui->SystemTableView->setColumnWidth(2, 130);
    ui->SystemTableView->setColumnWidth(3, 260);
    ui->SystemTableView->horizontalHeader()->setStretchLastSection(true);
    ui->SystemTableView->verticalHeader()->setVisible(false);

    facade->start();

    ui->SystemTableView->sortByColumn(static_cast<int>(Telemetry::Column::Timestamp),
                                      Qt::AscendingOrder);

    connect(ui->SystemTableView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int) { updateFollowMode(); });
    connect(ui->SystemTableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, [this](int, Qt::SortOrder) { updateFollowMode(); });

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

    connect(viewModel, &TelemetryViewModel::loadingStarted, this, [this]() {
        setLoadingOverlayVisible(true);
        setControlsEnabled(false);
    });
    connect(viewModel, &TelemetryViewModel::loadingFinished, this, [this, viewModel]() {
        setLoadingOverlayVisible(false);
        setControlsEnabled(true);
        updateFollowMode();
    });

    ui->tableStackedWidget->setCurrentWidget(ui->tablePage);

    const QDateTime now = QDateTime::currentDateTime();
    ui->filterDateFrom->setDate(now.date());
    ui->filterTimeFrom->setTime(QTime(0, 0));
    ui->filterDateTo->setDate(now.date());
    ui->filterTimeTo->setTime(now.time());

    bindStatisticsLabels();

    connect(ui->connectButton, &QPushButton::clicked,
            this, &MainWindow::onConnectButtonClicked);
    connect(ui->clearDatabaseButton, &QPushButton::clicked,
            this, &MainWindow::onClearDatabaseClicked);
    connect(ui->applyFilterButton, &QPushButton::clicked,
            this, &MainWindow::onApplyFilterClicked);
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

    onFilterFieldChanged(ui->filterFieldComboBox->currentIndex());
    normalizeNumericFilterInputs();
    updateFollowMode();
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

void MainWindow::onConnectButtonClicked()
{
    TelemetryViewModel *const viewModel = facade->telemetryViewModel();
    if (isGenerating) {
        facade->stopGeneration();
        setGenerationRunning(false);
        return;
    }

    viewModel->setLiveUpdatesEnabled(true);
    viewModel->setFilterMode(false);
    viewModel->setFollowLiveTail(true);
    facade->startGeneration();
    setGenerationRunning(true);
}

void MainWindow::onClearDatabaseClicked()
{
    facade->clearDatabase();
}

void MainWindow::syncFilterViewModelFromUi()
{
    FilterViewModel *const filterVm = facade->filterViewModel();
    const QDateTime fromDateTime =
        FilterViewModel::combineLocalDateTime(ui->filterDateFrom->date(), ui->filterTimeFrom->time());
    const QDateTime toDateTime =
        FilterViewModel::combineLocalDateTime(ui->filterDateTo->date(), ui->filterTimeTo->time());
    filterVm->setField(static_cast<FilterViewModel::Field>(ui->filterFieldComboBox->currentIndex()));
    filterVm->setSensorId(ui->filterSpinBox->value());
    filterVm->setValueFilter(
        ui->filterDoubleSpinBox->value(),
        ui->filterToleranceSpinBox->value(),
        static_cast<FilterViewModel::ValueOperation>(ui->filterValueOperationComboBox->currentIndex()));
    filterVm->setTimestampRange(fromDateTime, toDateTime);
}

void MainWindow::onApplyFilterClicked()
{
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

void MainWindow::setControlsEnabled(bool enabled)
{
    ui->SystemTableView->setEnabled(enabled);
    ui->SystemTableView->horizontalHeader()->setSectionsClickable(enabled);
    ui->applyFilterButton->setEnabled(enabled);
    ui->filterFieldComboBox->setEnabled(enabled);
    ui->filterValueStack->setEnabled(enabled);
    ui->clearDatabaseButton->setEnabled(enabled);
    ui->connectButton->setEnabled(enabled);
}
