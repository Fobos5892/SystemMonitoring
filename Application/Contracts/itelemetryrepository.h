#ifndef ITELEMETRYREPOSITORY_H
#define ITELEMETRYREPOSITORY_H

#include "Domain/sensordata.h"
#include "Domain/sensorstatistics.h"
#include "Domain/telemetrytypes.h"
#include <QObject>
#include <QString>
#include <QVector>

class ITelemetryRepository : public QObject {
    Q_OBJECT

public:
    explicit ITelemetryRepository(QObject *parent = nullptr) : QObject(parent) {}
    ~ITelemetryRepository() override = default;

public slots:
    virtual void initializeDatabase() = 0;
    virtual void shutdownDatabase() = 0;
    virtual void saveBatch(const QVector<SensorData> &batch) = 0;
    virtual void fetchSortedWindow(int sortColumn, int sortOrder, int limit) = 0;
    virtual void fetchSortedTail(int sortColumn, int sortOrder, int limit) = 0;
    virtual void fetchRangeNearAnchor(int sortColumn, int sortOrder, quint64 anchorRecordId,
                                    int limit, Telemetry::AnchorSide side) = 0;
    virtual void applyFilterQuery(const QString &filterCondition) = 0;
    virtual void clearDatabase() = 0;
    virtual void fetchSensorStatistics() = 0;

signals:
    void dataLoaded(const QVector<SensorData> &chunk);
    void tailDataLoaded(const QVector<SensorData> &chunk);
    void rangeNearAnchorLoaded(const QVector<SensorData> &chunk, Telemetry::AnchorSide side);
    void sensorStatisticsLoaded(const SensorStatistics &stats);
    void batchCommitted();
    void databaseCleared();
};

#endif // ITELEMETRYREPOSITORY_H
