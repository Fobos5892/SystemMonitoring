#include "dbdatacontroll.h"
#include "dbconnect.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DBDataControll::DBDataControll(const QString& url, QObject *parent)
    : QObject(parent)
    , m_connectionName("SQL_Worker_Connection")
    , m_dbManager(new DatabaseConnectionManager(url, m_connectionName))
{
    // Открываем базу данных и применяем PRAGMA-настройки скорости (WAL, synchronous=OFF)
    if (!m_dbManager->openConnection()) {
        qCritical() << "Не удалось инициализировать базу данных по пути:" << url;
    }
}

void DBDataControll::onSaveBatchToSql(const QVector<SensorData> &batch) {
    if (batch.isEmpty()) return;

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.transaction(); // Пакетная транзакция для высокой скорости

    QSqlQuery query(db);
    query.prepare("INSERT INTO telemetry (sensor_id, value, timestamp) VALUES (:sid, :val, :ts)");

    for (const auto &record : batch) {
        query.bindValue(":sid", record.id);
        query.bindValue(":val", record.value);
        query.bindValue(":ts", record.timestamp);
        query.exec();
    }
    db.commit();
}

void DBDataControll::fetchRangeBelow(unsigned long long bottomId, int count) {
    QVector<SensorData> result;
    result.reserve(count);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.setForwardOnly(true); // Отключаем кэш Qt, читаем строго вперед
    query.prepare("SELECT sensor_id, value, timestamp FROM telemetry "
                  "WHERE id < :bottomId ORDER BY id DESC LIMIT :limit");
    query.bindValue(":bottomId", bottomId);
    query.bindValue(":limit", count);

    if (query.exec()) {
        while (query.next()) {
            result.append({
                query.value(0).toULongLong(),
                query.value(2).toULongLong(),
                query.value(1).toDouble()
            });
        }
    }
    emit bottomChunkLoaded(result);
}

void DBDataControll::fetchRangeAbove(unsigned long long topId, int count) {
    QVector<SensorData> result;
    result.reserve(count);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.setForwardOnly(true);
    query.prepare("SELECT sensor_id, value, timestamp FROM telemetry "
                  "WHERE id > :topId ORDER BY id ASC LIMIT :limit");
    query.bindValue(":topId", topId);
    query.bindValue(":limit", count);

    if (query.exec()) {
        while (query.next()) {
            result.append({
                query.value(0).toULongLong(),
                query.value(2).toULongLong(),
                query.value(1).toDouble()
            });
        }
    }

    // Переворачиваем выборку в обратную сторону для хронологии в модели
    QVector<SensorData> reversedResult;
    reversedResult.reserve(result.size());
    for (int i = result.size() - 1; i >= 0; --i) {
        reversedResult.append(result[i]);
    }

    emit topChunkLoaded(reversedResult);
}

void DBDataControll::applyFilterQuery(const QString &filterCondition) {
    QVector<SensorData> result;
    result.reserve(500); // Ограничиваем размер первичного окна истории

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.setForwardOnly(true);

    QString queryString = "SELECT sensor_id, value, timestamp FROM telemetry";
    if (!filterCondition.isEmpty()) {
        queryString += " WHERE " + filterCondition;
    }
    queryString += " ORDER BY id DESC LIMIT 500";

    if (query.exec(queryString)) {
        while (query.next()) {
            result.append({
                query.value(0).toULongLong(),
                query.value(2).toULongLong(),
                query.value(1).toDouble()
            });
        }
    }
    // Передаем отфильтрованный срез данных как нижний чанк для инициализации модели
    emit bottomChunkLoaded(result);
}