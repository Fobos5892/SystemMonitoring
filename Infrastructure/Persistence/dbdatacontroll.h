#ifndef DBDATACONTROLL_H
#define DBDATACONTROLL_H

#include "dbconnect.h"
#include "Domain/filterqueryspec.h"
#include "Domain/sensordata.h"
#include "Domain/sensorstatistics.h"
#include "Domain/telemetrytypes.h"
#include <QScopedPointer>
#include <QVector>

class DatabaseConnectionManager;

class DBDataControll : public QObject {
    Q_OBJECT
public:
    static constexpr int SENSOR_ACTIVITY_WINDOW_MS = 10 * 60 * 1000; // 10 minutes

    DBDataControll() = delete;
    DBDataControll(const DBDataControll& value) = delete;
    DBDataControll& operator=(const DBDataControll& value) = delete;

    explicit DBDataControll(const QString& url, QObject *parent = nullptr);
    ~DBDataControll() = default;

public slots:
    void initializeDatabase();
    void shutdownDatabase();

    void onSaveBatchToSql(const QVector<SensorData> &batch);

    void fetchSortedWindow(int sortColumn, int sortOrder, int limit);
    void fetchSortedTail(int sortColumn, int sortOrder, int limit);
    void fetchRangeNearAnchor(int sortColumn, int sortOrder, quint64 anchorRecordId,
                              int limit, Telemetry::AnchorSide side);

    void applyFilterQuery(const FilterQuerySpec &filterSpec, int sortColumn, int sortOrder, int limit);
    void clearDatabase();
    void fetchSensorStatistics();

signals:
    void dataLoaded(const QVector<SensorData> &chunk);
    void tailDataLoaded(const QVector<SensorData> &chunk);
    void rangeNearAnchorLoaded(const QVector<SensorData> &chunk, Telemetry::AnchorSide side);
    void sensorStatisticsLoaded(const SensorStatistics &stats);
    void batchCommitted();
    void databaseCleared();

private:
    bool ensureSchema();
    void saveBatch(const QVector<SensorData> &batch);
    static QString sortColumnSql(int sortColumn);
    static SensorData readRow(const QSqlQuery &query);
    SensorStatistics loadSensorStatistics() const;
    QVector<SensorData> loadSortedWindowWithFilter(const FilterQuerySpec &filterSpec, int sortColumn,
                                                   int sortOrder, int limit) const;

    bool m_dbInitialized = false;
    bool m_hasActiveFilter = false;
    FilterQuerySpec m_activeFilterSpec;
    QScopedPointer<DatabaseConnectionManager> m_dbManager;
};

#endif // DBDATACONTROLL_H
