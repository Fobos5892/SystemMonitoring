#include "threadorchestrator.h"
#include "DeviceSimulator/devicesimulator.h"
#include "DeviceReceiver/devicereceiver.h"
#include "ViewModels/sensormodel.h"

ThreadOrchestrator::ThreadOrchestrator(SensorModel* uiModel, QObject *parent)
    : QObject(parent)
    , m_uiModel(uiModel)
    , m_simulator(new DeviceSimulator())
    , m_receiver(new DeviceReceiver())
    , m_dbController(new DBDataControll("telemetry.db"))
{
    m_filterRefreshTimer.setSingleShot(true);
    m_filterRefreshTimer.setInterval(300);
    connect(&m_filterRefreshTimer, &QTimer::timeout,
            this, &ThreadOrchestrator::onDebouncedFilterRefresh);
    initThreads();
    setupConnections();
}

ThreadOrchestrator::~ThreadOrchestrator() {
    stopAll();
}

void ThreadOrchestrator::initThreads() {
    m_simulator->moveToThread(&m_simulatorThread);
    m_receiver->moveToThread(&m_receiverThread);
    m_dbController->moveToThread(&m_sqlThread);
}

void ThreadOrchestrator::setupConnections() {
    setupSensorConnections();
    setupDatabaseOutputConnections();
    setupModelQueryConnections();
}

void ThreadOrchestrator::setupSensorConnections() {
    connect(m_simulator.data(), &DeviceSimulator::rawDataGenerated,
            m_receiver.data(), &DeviceReceiver::onRawDataReceived,
            Qt::QueuedConnection);

    connect(m_receiver.data(), &DeviceReceiver::dataBatchReady,
            m_dbController.data(), &DBDataControll::onSaveBatchToSql,
            Qt::QueuedConnection);
}

void ThreadOrchestrator::setupDatabaseOutputConnections() {
    connect(m_dbController.data(), &DBDataControll::batchCommitted,
            m_uiModel, &SensorModel::onBatchCommitted,
            Qt::QueuedConnection);
    connect(m_dbController.data(), &DBDataControll::batchCommitted,
            this, &ThreadOrchestrator::onBatchCommittedFromDb,
            Qt::QueuedConnection);

    connect(m_dbController.data(), &DBDataControll::dataLoaded,
            m_uiModel, &SensorModel::onDataLoaded,
            Qt::QueuedConnection);

    connect(m_dbController.data(), &DBDataControll::tailDataLoaded,
            m_uiModel, &SensorModel::onTailDataLoaded,
            Qt::QueuedConnection);

    connect(m_dbController.data(), &DBDataControll::rangeNearAnchorLoaded,
            m_uiModel, &SensorModel::onRangeNearAnchorLoaded,
            Qt::QueuedConnection);

    connect(m_dbController.data(), &DBDataControll::databaseCleared,
            m_uiModel, &SensorModel::onDatabaseCleared,
            Qt::QueuedConnection);

    connect(m_dbController.data(), &DBDataControll::sensorStatisticsLoaded,
            this, &ThreadOrchestrator::sensorStatisticsUpdated,
            Qt::QueuedConnection);
}

void ThreadOrchestrator::setupModelQueryConnections() {
    connect(m_uiModel, &SensorModel::sortRequested,
            this, &ThreadOrchestrator::onSortRequested);

    connect(m_uiModel, &SensorModel::tailRequest,
            this, &ThreadOrchestrator::onTailRequest);

    connect(m_uiModel, &SensorModel::rangeNearAnchorRequested,
            this, &ThreadOrchestrator::onRangeNearAnchorRequested);
}

void ThreadOrchestrator::startAll() {
    m_simulatorThread.start();
    m_receiverThread.start();
    m_sqlThread.start();

    QMetaObject::invokeMethod(m_dbController.data(), "initializeDatabase", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_receiver.data(), "startProcessing", Qt::QueuedConnection);
}

void ThreadOrchestrator::stopAll() {
    if (m_simulatorThread.isRunning()) {
        QMetaObject::invokeMethod(m_simulator.data(), "stopGeneration", Qt::BlockingQueuedConnection);
        m_simulatorThread.quit();
        m_simulatorThread.wait();
    }
    if (m_receiverThread.isRunning()) {
        QMetaObject::invokeMethod(m_receiver.data(), "stopProcessing", Qt::BlockingQueuedConnection);
        m_receiverThread.quit();
        m_receiverThread.wait();
    }
    if (m_sqlThread.isRunning()) {
        QMetaObject::invokeMethod(m_dbController.data(), "shutdownDatabase", Qt::BlockingQueuedConnection);
        m_sqlThread.quit();
        m_sqlThread.wait();
    }
}

void ThreadOrchestrator::onConnectRequested() {
    QMetaObject::invokeMethod(m_simulator.data(), "startGeneration", Qt::QueuedConnection);
}

void ThreadOrchestrator::onStopGenerationRequested() {
    QMetaObject::invokeMethod(m_simulator.data(), "stopGeneration", Qt::QueuedConnection);
}

void ThreadOrchestrator::onClearDatabaseRequested() {
    QMetaObject::invokeMethod(m_dbController.data(), "clearDatabase", Qt::QueuedConnection);
}

void ThreadOrchestrator::onFilterRequested(const QString &filterCondition) {
    m_filterActive = true;
    m_filterCondition = filterCondition;
    m_filterRefreshTimer.stop();
    m_uiModel->beginReloading();
    QMetaObject::invokeMethod(m_dbController.data(), "applyFilterQuery",
                              Qt::QueuedConnection, Q_ARG(QString, filterCondition));
}

void ThreadOrchestrator::onBatchCommittedFromDb() {
    if (!m_filterActive || m_filterCondition.isEmpty()) {
        return;
    }
    if (m_uiModel->isReloading()) {
        return;
    }

    // Пакетируем частые коммиты в редкие обновления UI, чтобы убрать мерцание.
    m_filterRefreshTimer.start();
}

void ThreadOrchestrator::onDebouncedFilterRefresh() {
    if (!m_filterActive || m_filterCondition.isEmpty() || m_uiModel->isReloading()) {
        return;
    }

    QMetaObject::invokeMethod(m_dbController.data(), "applyFilterQuery",
                              Qt::QueuedConnection, Q_ARG(QString, m_filterCondition));
}

void ThreadOrchestrator::onSortRequested(int column, int sortOrder) {
    if (m_filterActive) {
        // В режиме фильтра не сбрасываем условие сортировкой из view.
        m_filterRefreshTimer.stop();
        if (!m_uiModel->isReloading()) {
            m_uiModel->beginReloading();
        }
        QMetaObject::invokeMethod(m_dbController.data(), "applyFilterQuery",
                                  Qt::QueuedConnection, Q_ARG(QString, m_filterCondition));
        return;
    }

    m_filterActive = false;
    m_filterCondition.clear();
    m_filterRefreshTimer.stop();
    QMetaObject::invokeMethod(m_dbController.data(), "fetchSortedWindow",
                              Qt::QueuedConnection,
                              Q_ARG(int, column),
                              Q_ARG(int, sortOrder),
                              Q_ARG(int, 500));
}

void ThreadOrchestrator::onTailRequest(int sortColumn, int sortOrder, int limit) {
    QMetaObject::invokeMethod(m_dbController.data(), "fetchSortedTail",
                              Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(int, limit));
}

void ThreadOrchestrator::onRangeNearAnchorRequested(int sortColumn, int sortOrder,
                                                    quint64 anchorRecordId, int limit,
                                                    DBDataControll::AnchorSide side) {
    QMetaObject::invokeMethod(m_dbController.data(), "fetchRangeNearAnchor",
                              Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(quint64, anchorRecordId),
                              Q_ARG(int, limit),
                              Q_ARG(DBDataControll::AnchorSide, side));
}
