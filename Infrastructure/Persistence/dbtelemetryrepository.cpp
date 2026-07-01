#include "dbtelemetryrepository.h"
#include "dbdatacontroll.h"

DbTelemetryRepository::DbTelemetryRepository(DBDataControll *dataController, QObject *parent)
    : ITelemetryRepository(parent)
    , dataController(dataController)
{
    connect(dataController, &DBDataControll::dataLoaded,
            this, &DbTelemetryRepository::dataLoaded);
    connect(dataController, &DBDataControll::tailDataLoaded,
            this, &DbTelemetryRepository::tailDataLoaded);
    connect(dataController, &DBDataControll::rangeNearAnchorLoaded,
            this, &DbTelemetryRepository::rangeNearAnchorLoaded);
    connect(dataController, &DBDataControll::sensorStatisticsLoaded,
            this, &DbTelemetryRepository::sensorStatisticsLoaded);
    connect(dataController, &DBDataControll::batchCommitted,
            this, &DbTelemetryRepository::batchCommitted);
    connect(dataController, &DBDataControll::databaseCleared,
            this, &DbTelemetryRepository::databaseCleared);
}

void DbTelemetryRepository::initializeDatabase()
{
    QMetaObject::invokeMethod(dataController, "initializeDatabase", Qt::QueuedConnection);
}

void DbTelemetryRepository::shutdownDatabase()
{
    QMetaObject::invokeMethod(dataController, "shutdownDatabase", Qt::QueuedConnection);
}

void DbTelemetryRepository::saveBatch(const QVector<SensorData> &batch)
{
    QMetaObject::invokeMethod(dataController, "onSaveBatchToSql", Qt::QueuedConnection,
                              Q_ARG(QVector<SensorData>, batch));
}

void DbTelemetryRepository::fetchSortedWindow(int sortColumn, int sortOrder, int limit)
{
    QMetaObject::invokeMethod(dataController, "fetchSortedWindow", Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(int, limit));
}

void DbTelemetryRepository::fetchSortedTail(int sortColumn, int sortOrder, int limit)
{
    QMetaObject::invokeMethod(dataController, "fetchSortedTail", Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(int, limit));
}

void DbTelemetryRepository::fetchRangeNearAnchor(int sortColumn, int sortOrder,
                                                 quint64 anchorRecordId, int limit,
                                                 Telemetry::AnchorSide side)
{
    QMetaObject::invokeMethod(dataController, "fetchRangeNearAnchor", Qt::QueuedConnection,
                              Q_ARG(int, sortColumn),
                              Q_ARG(int, sortOrder),
                              Q_ARG(quint64, anchorRecordId),
                              Q_ARG(int, limit),
                              Q_ARG(Telemetry::AnchorSide, side));
}

void DbTelemetryRepository::applyFilterQuery(const QString &filterCondition)
{
    QMetaObject::invokeMethod(dataController, "applyFilterQuery", Qt::QueuedConnection,
                              Q_ARG(QString, filterCondition));
}

void DbTelemetryRepository::clearDatabase()
{
    QMetaObject::invokeMethod(dataController, "clearDatabase", Qt::QueuedConnection);
}

void DbTelemetryRepository::fetchSensorStatistics()
{
    QMetaObject::invokeMethod(dataController, "fetchSensorStatistics", Qt::QueuedConnection);
}
