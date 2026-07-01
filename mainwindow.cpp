#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ThreadOrchestrator/threadorchestrator.h"
#include <QDateTime>
#include <QHeaderView>
#include <QSignalBlocker>
#include <QScrollBar>
#include <QStyle>
#include <algorithm>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_model(new SensorModel(this))
    , m_orchestrator(new ThreadOrchestrator(m_model, this))
{
    ui->setupUi(this);

    ui->SystemTableView->setModel(m_model);
    ui->SystemTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->SystemTableView->horizontalHeader()->setMinimumSectionSize(60);
    ui->SystemTableView->setColumnWidth(0, 90);   // № записи
    ui->SystemTableView->setColumnWidth(1, 90);   // №датчика
    ui->SystemTableView->setColumnWidth(2, 130);  // Значение
    ui->SystemTableView->setColumnWidth(3, 260);  // Дата и Время
    ui->SystemTableView->horizontalHeader()->setStretchLastSection(true);
    ui->SystemTableView->verticalHeader()->setVisible(false);

    m_orchestrator->startAll();

    ui->SystemTableView->sortByColumn(static_cast<int>(SensorModel::Column::Timestamp),
                                      Qt::AscendingOrder);

    auto updateFollowMode = [this]() {
        const QScrollBar *scrollBar = ui->SystemTableView->verticalScrollBar();
        const bool atTop = scrollBar->value() <= scrollBar->minimum();
        const bool atBottom = scrollBar->value() >= scrollBar->maximum();
        const Qt::SortOrder order = ui->SystemTableView->horizontalHeader()->sortIndicatorOrder();
        m_model->setFollowLiveTail(order == Qt::AscendingOrder ? atBottom : atTop);
    };

    connect(ui->SystemTableView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [updateFollowMode](int) { updateFollowMode(); });

    connect(ui->SystemTableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, [updateFollowMode](int, Qt::SortOrder) { updateFollowMode(); });

    connect(m_model, &SensorModel::liveDataInserted, this, [this]() {
        if (!m_model->isFollowingLiveTail()) {
            return;
        }
        const Qt::SortOrder order = ui->SystemTableView->horizontalHeader()->sortIndicatorOrder();
        if (order == Qt::AscendingOrder) {
            ui->SystemTableView->scrollToBottom();
        } else {
            ui->SystemTableView->scrollToTop();
        }
    });

    connect(m_model, &SensorModel::loadingStarted, this, [this]() {
        setLoadingOverlayVisible(true);
        setControlsEnabled(false);
    });
    connect(m_model, &SensorModel::loadingFinished, this, [this]() {
        setLoadingOverlayVisible(false);
        setControlsEnabled(true);
        m_model->setFollowLiveTail(true);
        const Qt::SortOrder order = ui->SystemTableView->horizontalHeader()->sortIndicatorOrder();
        if (order == Qt::AscendingOrder) {
            ui->SystemTableView->scrollToBottom();
        } else {
            ui->SystemTableView->scrollToTop();
        }
    });

    ui->tableStackedWidget->setCurrentWidget(ui->tablePage);

    const QDateTime now = QDateTime::currentDateTime();
    ui->filterDateFrom->setDateTime(now.date().startOfDay());
    ui->filterDateTo->setDateTime(now);

    connect(m_orchestrator, &ThreadOrchestrator::sensorStatisticsUpdated,
            this, &MainWindow::onSensorStatisticsUpdated);

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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectButtonClicked()
{
    if (m_isGenerating) {
        m_orchestrator->onStopGenerationRequested();
        setGenerationRunning(false);
        return;
    }

    m_model->setLiveUpdatesEnabled(true);
    m_model->setFilterMode(false);
    m_model->setFollowLiveTail(true);
    m_orchestrator->onConnectRequested();
    setGenerationRunning(true);
}

void MainWindow::onClearDatabaseClicked()
{
    m_orchestrator->onClearDatabaseRequested();
}

void MainWindow::onApplyFilterClicked()
{
    m_model->setLiveUpdatesEnabled(false);
    m_model->setFilterMode(true);
    m_model->setFollowLiveTail(false);
    m_orchestrator->onFilterRequested(buildFilterCondition());
}

void MainWindow::onFilterFieldChanged(int index)
{
    ui->filterValueStack->setCurrentIndex(index);
}

void MainWindow::onSensorStatisticsUpdated(const SensorStatistics &stats)
{
    ui->statsConnectedLabel->setText(
        tr("Активных датчиков: %1").arg(stats.connectedCount()));
    ui->statsAverageLabel->setText(
        tr("Среднее: %1 В").arg(stats.averageValue(), 0, 'f', 2));
    ui->statsMinLabel->setText(
        tr("Минимум: %1 В").arg(stats.minimumValue(), 0, 'f', 2));
    ui->statsMaxLabel->setText(
        tr("Максимум: %1 В").arg(stats.maximumValue(), 0, 'f', 2));
}

void MainWindow::normalizeNumericFilterInputs()
{
    auto truncateTo = [](double value, int decimals) -> double {
        const double scale = std::pow(10.0, decimals);
        return std::floor(value * scale) / scale;
    };

    const double normalizedValue = truncateTo(std::clamp(ui->filterDoubleSpinBox->value(), 0.0, 280.0), 2);
    const double normalizedTolerance =
        truncateTo(std::clamp(ui->filterToleranceSpinBox->value(), 0.0, 1.0), 4);

    const QSignalBlocker valueBlocker(ui->filterDoubleSpinBox);
    const QSignalBlocker toleranceBlocker(ui->filterToleranceSpinBox);
    ui->filterDoubleSpinBox->setValue(normalizedValue);
    ui->filterToleranceSpinBox->setValue(normalizedTolerance);

    // Для мелких значений удобнее точный шаг, для крупных — быстрый.
    const double adaptiveStep = normalizedTolerance < 0.01 ? 0.0001 : 0.01;
    ui->filterToleranceSpinBox->setSingleStep(adaptiveStep);
}

QString MainWindow::buildFilterCondition() const
{
    auto truncateTo = [](double value, int decimals) -> double {
        const double scale = std::pow(10.0, decimals);
        return std::floor(value * scale) / scale;
    };

    switch (ui->filterFieldComboBox->currentIndex()) {
    case 0:
        return QStringLiteral("sensor_id = %1").arg(ui->filterSpinBox->value());
    case 1: {
        const double rawValue = ui->filterDoubleSpinBox->value();
        const double rawTolerance = ui->filterToleranceSpinBox->value();
        const double value = truncateTo(std::clamp(rawValue, 0.0, 280.0), 2);
        const double tolerance = truncateTo(std::clamp(rawTolerance, 0.0, 1.0), 4);
        const QString valueSql = QString::number(value, 'f', 2);
        const QString toleranceSql = QString::number(tolerance, 'f', 4);

        switch (ui->filterValueOperationComboBox->currentIndex()) {
        case 0:
            return QStringLiteral("ABS(value - %1) <= %2").arg(valueSql, toleranceSql);
        case 1:
            return QStringLiteral("value > %1")
                .arg(QString::number(value + tolerance, 'f', 4));
        case 2:
            return QStringLiteral("value < %1")
                .arg(QString::number(value - tolerance, 'f', 4));
        default:
            return QStringLiteral("ABS(value - %1) <= %2").arg(valueSql, toleranceSql);
        }
    }
    case 2: {
        const qint64 fromMs = ui->filterDateFrom->dateTime().toMSecsSinceEpoch();
        const qint64 toMs = ui->filterDateTo->dateTime().toMSecsSinceEpoch();
        const qint64 startMs = qMin(fromMs, toMs);
        const qint64 endMs = qMax(fromMs, toMs);
        return QStringLiteral("timestamp BETWEEN %1 AND %2").arg(startMs).arg(endMs);
    }
    default:
        return {};
    }
}

void MainWindow::setGenerationRunning(bool running)
{
    m_isGenerating = running;
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
