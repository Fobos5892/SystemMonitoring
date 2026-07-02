#include "dbtelemetryrepository.h"
#include "dbdatacontroll.h"

DbTelemetryRepository::DbTelemetryRepository(DBDataControll *dataController, QObject *parent)
    : ITelemetryRepository(parent)
    , dataController(dataController)
{
    connect(this->dataController.data(), &DBDataControll::dataLoaded,
            this, &DbTelemetryRepository::dataLoaded);
    connect(this->dataController.data(), &DBDataControll::tailDataLoaded,
            this, &DbTelemetryRepository::tailDataLoaded);
    connect(this->dataController.data(), &DBDataControll::rangeNearAnchorLoaded,
            this, &DbTelemetryRepository::rangeNearAnchorLoaded);
    connect(this->dataController.data(), &DBDataControll::sensorStatisticsLoaded,
            this, &DbTelemetryRepository::sensorStatisticsLoaded);
    connect(this->dataController.data(), &DBDataControll::batchCommitted,
            this, &DbTelemetryRepository::batchCommitted);
    connect(this->dataController.data(), &DBDataControll::databaseCleared,
            this, &DbTelemetryRepository::databaseCleared);
}

void DbTelemetryRepository::initializeDatabase()
{
    QMetaObject::invokeMethod(dataController.data(), "initializeDatabase", Qt::QueuedConnection);
}

void DbTelemetryRepository::shutdownDatabase()
{
    QMetaObject::invokeMethod(dataController.data(), "shutdownDatabase", Qt::QueuedConnection);
}

void DbTelemetryRepository::saveBatch(const QVector<SensorData> &batch)
{
    QMetaObject::invokeMethod(dataController.data(), "onSaveBatchToSql", Qt::QueuedConnection,
                              Q_ARG(QVector<SensorData>, batch));
}

void DbTelemetryRepository::fetchSortedWindow(int sortColumn, int sortOrder, int limit)
{
    QMetaObject::invokeMethod(dataController.data(), "fetchSortedWindow", Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(int, limit));
}

void DbTelemetryRepository::fetchSortedTail(int sortColumn, int sortOrder, int limit)
{
    QMetaObject::invokeMethod(dataController.data(), "fetchSortedTail", Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(int, limit));
}

void DbTelemetryRepository::fetchRangeNearAnchor(int sortColumn, int sortOrder,
                                                 quint64 anchorRecordId, int limit,
                                                 Telemetry::AnchorSide side)
{
    QMetaObject::invokeMethod(dataController.data(), "fetchRangeNearAnchor", Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(quint64, anchorRecordId),
                              Q_ARG(int, limit),
                              Q_ARG(Telemetry::AnchorSide, side));
}

void DbTelemetryRepository::applyFilterQuery(const FilterQuerySpec &filterSpec, int sortColumn,
                                             int sortOrder, int limit)
{
    QMetaObject::invokeMethod(this, "runFilterQuery", Qt::QueuedConnection,
                              Q_ARG(FilterQuerySpec, filterSpec),
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(int, limit));
}

void DbTelemetryRepository::runFilterQuery(FilterQuerySpec filterSpec, int sortColumn,
                                           int sortOrder, int limit)
{
    dataController->applyFilterQuery(filterSpec, sortColumn, sortOrder, limit);
}

void DbTelemetryRepository::clearDatabase()
{
    QMetaObject::invokeMethod(dataController.data(), "clearDatabase", Qt::QueuedConnection);
}

void DbTelemetryRepository::fetchSensorStatistics()
{
    QMetaObject::invokeMethod(dataController.data(), "fetchSensorStatistics", Qt::QueuedConnection);
}
