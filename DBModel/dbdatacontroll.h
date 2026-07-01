#ifndef DBDATACONTROLL_H
#define DBDATACONTROLL_H

#include "dbconnect.h"
#include "Data/sensordata.h"
#include <QScopedPointer>
#include <QVector>

class DatabaseConnectionManager;

class DBDataControll : public QObject {
    Q_OBJECT
public:
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
    void fetchSortedTailWindow(int sortColumn, int sortOrder, int limit);
    void fetchRangeAfterAnchor(int sortColumn, int sortOrder, quint64 anchorRecordId, int limit);
    void fetchRangeBeforeAnchor(int sortColumn, int sortOrder, quint64 anchorRecordId, int limit);

    void applyFilterQuery(const QString &filterCondition);
    void clearDatabase();

signals:
    void windowDataLoaded(const QVector<SensorData> &chunk);
    void tailWindowLoaded(const QVector<SensorData> &chunk);
    void rangeAfterAnchorLoaded(const QVector<SensorData> &chunk);
    void rangeBeforeAnchorLoaded(const QVector<SensorData> &chunk);
    void batchCommitted();
    void databaseCleared();

private:
    bool ensureSchema();
    void saveBatch(const QVector<SensorData> &batch);
    void flushPendingBatches();
    static QString sortColumnSql(int sortColumn);
    static SensorData readRow(const QSqlQuery &query);

    QString m_connectionName;
    bool m_dbInitialized = false;
    bool m_writesBlocked = false;
    QVector<QVector<SensorData>> m_pendingBatches;
    QScopedPointer<DatabaseConnectionManager> m_dbManager;
};

#endif // DBDATACONTROLL_H
