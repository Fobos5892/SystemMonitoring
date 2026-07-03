#ifndef ITELEMETRYREPOSITORY_H
#define ITELEMETRYREPOSITORY_H

#include "Domain/sensordata.h"
#include "Domain/sensordatabatch.h"
#include "Domain/sensorstatistics.h"
#include "Domain/filterqueryspec.h"
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
    virtual void saveBatch(SensorDataBatch batch) = 0;
    virtual void fetchSortedWindow(int sortColumn, int sortOrder, int limit) = 0;
    virtual void fetchSortedTail(int sortColumn, int sortOrder, int limit) = 0;
    virtual void fetchRangeNearAnchor(int sortColumn, int sortOrder, quint64 anchorRecordId,
                                    int limit, Telemetry::AnchorSide side) = 0;
    virtual void applyFilterQuery(const FilterQuerySpec &filterSpec, int sortColumn,
                                  int sortOrder, int limit) = 0;
    virtual void clearDatabase() = 0;
    virtual void fetchSensorStatistics() = 0;

signals:
    void dataLoaded(SensorDataBatch chunk);
    void tailDataLoaded(SensorDataBatch chunk);
    void rangeNearAnchorLoaded(SensorDataBatch chunk, Telemetry::AnchorSide side);
    void sensorStatisticsLoaded(const SensorStatistics &stats);
    void batchCommitted(SensorDataBatch inserted);
    void databaseCleared();
};

#endif // ITELEMETRYREPOSITORY_H
