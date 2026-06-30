#ifndef DBDATACONTROLL_H
#define DBDATACONTROLL_H

#include "dbconnect.h"
#include "Data/sensordata.h"
#include <QScopedPointer>

class DatabaseConnectionManager;

class DBDataControll : public QObject {
    Q_OBJECT
public:
    DBDataControll() = delete;
    DBDataControll(const DBDataControll& value) = delete;
    DBDataControll& operator=(const DBDataControll& value) = delete;

    explicit DBDataControll(const QString& url, QObject *parent = nullptr);
    ~DBDataControll() = default; // QScopedPointer сам очистит dbManager

public slots:
    // Слот для асинхронного сохранения пакетов от устройств
    void onSaveBatchToSql(const QVector<SensorData> &batch);

    // Слоты для асинхронной пагинации скользящего окна
    void fetchRangeBelow(unsigned long long bottomId, int count);
    void fetchRangeAbove(unsigned long long topId, int count);

    // Слот для тяжелых SQL-запросов фильтрации (по кнопке "Применить фильтр")
    void applyFilterQuery(const QString &filterCondition);

signals:
    // Сигналы обратной связи (уходят в SensorModel через оркестратор)
    void bottomChunkLoaded(const QVector<SensorData> &chunk);
    void topChunkLoaded(const QVector<SensorData> &chunk);

private:
    QString m_connectionName;
    QScopedPointer<DatabaseConnectionManager> m_dbManager;
};

#endif // DBDATACONTROLL_H
