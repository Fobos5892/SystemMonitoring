#ifndef DBTELEMETRYREPOSITORY_H
#define DBTELEMETRYREPOSITORY_H

#include "Application/Contracts/itelemetryrepository.h"
#include "dbdatacontroll.h"

#include <QScopedPointer>

class DbTelemetryRepository : public ITelemetryRepository {
    Q_OBJECT

public:
    explicit DbTelemetryRepository(DBDataControll *dataController, QObject *parent = nullptr);

    DBDataControll *controller() const { return dataController.data(); }

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
    QScopedPointer<DBDataControll> dataController;
};

#endif // DBTELEMETRYREPOSITORY_H
