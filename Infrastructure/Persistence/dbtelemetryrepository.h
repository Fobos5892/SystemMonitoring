#ifndef DBTELEMETRYREPOSITORY_H
#define DBTELEMETRYREPOSITORY_H

#include "Application/Contracts/itelemetryrepository.h"

class DBDataControll;

class DbTelemetryRepository : public ITelemetryRepository {
    Q_OBJECT

public:
    explicit DbTelemetryRepository(DBDataControll *dataController, QObject *parent = nullptr);

public slots:
    void initializeDatabase() override;
    void shutdownDatabase() override;
    void saveBatch(const QVector<SensorData> &batch) override;
    void fetchSortedWindow(int sortColumn, int sortOrder, int limit) override;
    void fetchSortedTail(int sortColumn, int sortOrder, int limit) override;
    void fetchRangeNearAnchor(int sortColumn, int sortOrder, quint64 anchorRecordId,
                              int limit, Telemetry::AnchorSide side) override;
    void applyFilterQuery(const QString &filterCondition) override;
    void clearDatabase() override;
    void fetchSensorStatistics() override;

private:
    DBDataControll *dataController = nullptr;
};

#endif // DBTELEMETRYREPOSITORY_H
