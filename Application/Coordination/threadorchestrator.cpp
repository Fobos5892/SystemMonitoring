#include "threadorchestrator.h"
#include "Infrastructure/Devices/devicesimulator.h"
#include "Infrastructure/Devices/devicereceiver.h"
#include "Infrastructure/Persistence/dbdatacontroll.h"
#include "Infrastructure/Persistence/dbtelemetryrepository.h"
#include "ViewModels/telemetryviewmodel.h"
#include <QtGlobal>

ThreadOrchestrator::ThreadOrchestrator(TelemetryViewModel *viewModel, QObject *parent)
    : QObject(parent)
    , viewModel(viewModel)
    , simulator(new DeviceSimulator())
    , receiver(new DeviceReceiver())
    , repositoryAdapter(new DbTelemetryRepository(new DBDataControll("telemetry.db")))
{
    repository = repositoryAdapter.data();

    filterRefreshTimer.setSingleShot(true);
    filterRefreshTimer.setInterval(FILTER_REFRESH_DEBOUNCE_MS);
    connect(&filterRefreshTimer, &QTimer::timeout,
            this, &ThreadOrchestrator::onDebouncedFilterRefresh);
    initThreads();
    setupConnections();
}

ThreadOrchestrator::~ThreadOrchestrator() {
    stopAll();
}

void ThreadOrchestrator::initThreads() {
    simulator->moveToThread(&simulatorThread);
    receiver->moveToThread(&receiverThread);
    repositoryAdapter->controller()->moveToThread(&sqlThread);
    repositoryAdapter->moveToThread(&sqlThread);
}

void ThreadOrchestrator::setupConnections() {
    setupSensorConnections();
    setupRepositoryOutputConnections();
    setupViewModelQueryConnections();
}

void ThreadOrchestrator::setupSensorConnections() {
    connect(simulator.data(), &DeviceSimulator::rawDataGenerated,
            receiver.data(), &DeviceReceiver::onRawDataReceived,
            Qt::QueuedConnection);

    connect(receiver.data(), &DeviceReceiver::dataBatchReady,
            repositoryAdapter.data(), &DbTelemetryRepository::saveBatch,
            Qt::QueuedConnection);
}

void ThreadOrchestrator::setupRepositoryOutputConnections() {
    connect(repository, &ITelemetryRepository::batchCommitted,
            viewModel, &TelemetryViewModel::onBatchCommitted,
            Qt::QueuedConnection);
    connect(repository, &ITelemetryRepository::batchCommitted,
            this, &ThreadOrchestrator::onBatchCommittedFromDb,
            Qt::QueuedConnection);

    connect(repository, &ITelemetryRepository::dataLoaded,
            viewModel, &TelemetryViewModel::onDataLoaded,
            Qt::QueuedConnection);

    connect(repository, &ITelemetryRepository::tailDataLoaded,
            viewModel, &TelemetryViewModel::onTailDataLoaded,
            Qt::QueuedConnection);

    connect(repository, &ITelemetryRepository::rangeNearAnchorLoaded,
            viewModel, &TelemetryViewModel::onRangeNearAnchorLoaded,
            Qt::QueuedConnection);

    connect(repository, &ITelemetryRepository::databaseCleared,
            viewModel, &TelemetryViewModel::onDatabaseCleared,
            Qt::QueuedConnection);

    connect(repository, &ITelemetryRepository::sensorStatisticsLoaded,
            this, &ThreadOrchestrator::sensorStatisticsUpdated,
            Qt::QueuedConnection);
}

void ThreadOrchestrator::setupViewModelQueryConnections() {
    connect(viewModel, &TelemetryViewModel::sortRequested,
            this, &ThreadOrchestrator::onSortRequested);

    connect(viewModel, &TelemetryViewModel::tailRequest,
            this, &ThreadOrchestrator::onTailRequest);

    connect(viewModel, &TelemetryViewModel::rangeNearAnchorRequested,
            this, &ThreadOrchestrator::onRangeNearAnchorRequested);
}

void ThreadOrchestrator::startAll() {
    simulatorThread.start();
    receiverThread.start();
    sqlThread.start();

    repository->initializeDatabase();
    QMetaObject::invokeMethod(receiver.data(), "startProcessing", Qt::QueuedConnection);
}

void ThreadOrchestrator::stopAll() {
    if (simulatorThread.isRunning()) {
        QMetaObject::invokeMethod(simulator.data(), "stopGeneration", Qt::BlockingQueuedConnection);
        simulatorThread.quit();
        simulatorThread.wait();
    }
    if (receiverThread.isRunning()) {
        QMetaObject::invokeMethod(receiver.data(), "stopProcessing", Qt::BlockingQueuedConnection);
        receiverThread.quit();
        receiverThread.wait();
    }
    if (sqlThread.isRunning()) {
        QMetaObject::invokeMethod(repositoryAdapter->controller(), "shutdownDatabase",
                                  Qt::BlockingQueuedConnection);
        sqlThread.quit();
        sqlThread.wait();
    }
}

void ThreadOrchestrator::onConnectRequested() {
    QMetaObject::invokeMethod(simulator.data(), "startGeneration", Qt::QueuedConnection);
}

void ThreadOrchestrator::onStopGenerationRequested() {
    QMetaObject::invokeMethod(simulator.data(), "stopGeneration", Qt::QueuedConnection);
}

void ThreadOrchestrator::onClearDatabaseRequested() {
    repository->clearDatabase();
}

void ThreadOrchestrator::onFilterRequested(const FilterQuerySpec &filterSpec,
                                           int sortColumn, int sortOrder, int limit) {
    filterActive = true;
    currentSortColumn = sortColumn;
    currentSortOrder = sortOrder;
    this->filterSpec = filterSpec;
    filterLimit = qMax(limit, Telemetry::CHUNK_SIZE);
    filterRefreshTimer.stop();
    viewModel->beginReloading(filterLimit);
    repository->applyFilterQuery(filterSpec, currentSortColumn, currentSortOrder, filterLimit);
}

void ThreadOrchestrator::onBatchCommittedFromDb() {
    if (!filterActive) {
        return;
    }
    if (viewModel->isReloading() || !viewModel->isFilterMode()) {
        return;
    }

    filterRefreshTimer.start();
}

void ThreadOrchestrator::onDebouncedFilterRefresh() {
    if (!filterActive || viewModel->isReloading()) {
        return;
    }

    repository->applyFilterQuery(filterSpec, currentSortColumn, currentSortOrder, filterLimit);
}

void ThreadOrchestrator::onSortRequested(int column, int sortOrder) {
    currentSortColumn = column;
    currentSortOrder = sortOrder;
    if (filterActive) {
        filterRefreshTimer.stop();
        if (!viewModel->isReloading()) {
            viewModel->beginReloading(filterLimit);
        }
        repository->applyFilterQuery(filterSpec, column, sortOrder, filterLimit);
        return;
    }

    filterActive = false;
    filterRefreshTimer.stop();
    repository->fetchSortedWindow(column, sortOrder, Telemetry::WINDOW_SIZE);
}

void ThreadOrchestrator::onTailRequest(int sortColumn, int sortOrder, int limit) {
    repository->fetchSortedTail(sortColumn, sortOrder, limit);
}

void ThreadOrchestrator::onRangeNearAnchorRequested(int sortColumn, int sortOrder,
                                                    quint64 anchorRecordId, int limit,
                                                    Telemetry::AnchorSide side) {
    repository->fetchRangeNearAnchor(sortColumn, sortOrder, anchorRecordId, limit, side);
}
