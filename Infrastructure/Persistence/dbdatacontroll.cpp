#include "dbdatacontroll.h"
#include "dbconnect.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <algorithm>

namespace {

SensorData readSensorRow(const QSqlQuery &query) {
    return {
        query.value(0).toULongLong(),
        query.value(1).toULongLong(),
        query.value(3).toULongLong(),
        query.value(2).toDouble()
    };
}

/**
 * WHERE-условие для выборки записей рядом с якорем.
 * @column - колонка сортировки (без префикса t.)
 * @ascending - порядок сортировки таблицы
 * @side - Top: строки выше якоря, Bottom: строки ниже якоря
 */
QString getNewDataNearAnchor(const QString &column, bool ascending, Telemetry::AnchorSide side) {
    const bool useGreater = (side == Telemetry::AnchorSide::Bottom) == ascending;
    if (useGreater) {
        return QStringLiteral("(t.%1 > a.%1) OR (t.%1 = a.%1 AND t.id > a.id)").arg(column);
    }
    return QStringLiteral("(t.%1 < a.%1) OR (t.%1 = a.%1 AND t.id < a.id)").arg(column);
}

/**
 * ORDER BY для range-запросов относительно якорной строки.
 * @column - имя колонки сортировки (без префикса t.)
 * @ascending - порядок сортировки таблицы
 * @side - Top: строки выше якоря, Bottom: строки ниже якоря
 */
QString orderByClause(const QString &column, bool ascending, Telemetry::AnchorSide side) {
    const bool orderAscending = side == Telemetry::AnchorSide::Bottom ? ascending : !ascending;
    const QString direction = orderAscending ? QStringLiteral("ASC") : QStringLiteral("DESC");
    return QStringLiteral("t.%1 %2, t.id %2").arg(column, direction);
}

/**
 * Загружает порцию записей рядом с якорной строкой.
 * @anchorRecordId - id якорной записи в telemetry
 * @limit - максимальное число строк в ответе
 * @column - колонка сортировки (без префикса t.)
 * @ascending - порядок сортировки таблицы
 * @side - Top: строки выше якоря, Bottom: строки ниже якоря
 */
QVector<SensorData> loadRangeNearAnchor(quint64 anchorRecordId, int limit,
                                        const QString &column, bool ascending,
                                        Telemetry::AnchorSide side,
                                        const QString &filterSqlCondition) {
    QVector<SensorData> result;
    result.reserve(limit);

    QSqlQuery query(QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)));
    query.setForwardOnly(true);
    const QString filterClause = filterSqlCondition.isEmpty()
        ? QString()
        : QStringLiteral(" AND (%1)").arg(filterSqlCondition);
    query.prepare(QStringLiteral(
        "SELECT t.id, t.sensor_id, t.value, t.timestamp "
        "FROM telemetry t, telemetry a "
        "WHERE a.id = :anchorId AND (%1)%2 "
        "ORDER BY %3 LIMIT :limit")
                      .arg(getNewDataNearAnchor(column, ascending, side),
                           filterClause,
                           orderByClause(column, ascending, side)));
    query.bindValue(":anchorId", anchorRecordId);
    query.bindValue(":limit", limit);

    if (query.exec()) {
        while (query.next()) {
            result.append(readSensorRow(query));
        }
    } else {
        qCritical() << "Ошибка range-запроса в SQL:" << query.lastError().text();
    }

    if (side == Telemetry::AnchorSide::Top) {
        std::reverse(result.begin(), result.end());
    }
    return result;
}

} // namespace

DBDataControll::DBDataControll(const QString& url, QObject *parent)
    : QObject(parent)
    , m_dbManager(new DatabaseConnectionManager(
          url, QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)))
{
}

void DBDataControll::initializeDatabase() {
    if (m_dbInitialized) {
        return;
    }

    if (!m_dbManager->openConnection()) {
        qCritical() << "Не удалось открыть БД в SQL-потоке";
        return;
    }

    if (!ensureSchema()) {
        qCritical() << "Не удалось создать схему таблицы telemetry";
        m_dbManager->closeConnection();
        return;
    }

    m_dbInitialized = true;
    fetchSensorStatistics();
}

void DBDataControll::shutdownDatabase() {
    if (!m_dbInitialized) {
        return;
    }

    m_dbManager->closeConnection();
    m_dbInitialized = false;
}

bool DBDataControll::ensureSchema() {
    QSqlQuery query(QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)));

    if (!query.exec(
            "CREATE TABLE IF NOT EXISTS telemetry ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  sensor_id INTEGER NOT NULL,"
            "  value REAL NOT NULL,"
            "  timestamp INTEGER NOT NULL"
            ")")) {
        qCritical() << "Не удалось создать таблицу telemetry:" << query.lastError().text();
        return false;
    }

    const QStringList indexStatements = {
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_telemetry_sensor_id ON telemetry(sensor_id)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_telemetry_value ON telemetry(value)"),
        QStringLiteral("CREATE INDEX IF NOT EXISTS idx_telemetry_timestamp ON telemetry(timestamp)")
    };

    for (const QString &statement : indexStatements) {
        if (!query.exec(statement)) {
            qCritical() << "Не удалось создать индекс:" << query.lastError().text();
            return false;
        }
    }

    return true;
}

QString DBDataControll::sortColumnSql(int sortColumn) {
    switch (sortColumn) {
    case 0: return QStringLiteral("id");
    case 1: return QStringLiteral("sensor_id");
    case 2: return QStringLiteral("value");
    case 3: return QStringLiteral("timestamp");
    default: return QStringLiteral("timestamp");
    }
}

SensorData DBDataControll::readRow(const QSqlQuery &query) {
    return {
        query.value(0).toULongLong(),
        query.value(1).toULongLong(),
        query.value(3).toULongLong(),
        query.value(2).toDouble()
    };
}

void DBDataControll::saveBatch(const QVector<SensorData> &batch) {
    if (batch.isEmpty() || !m_dbInitialized) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME));
    db.transaction();

    QSqlQuery query(db);
    query.prepare("INSERT INTO telemetry (sensor_id, value, timestamp) VALUES (:sid, :val, :ts)");

    for (const auto &record : batch) {
        query.bindValue(":sid", record.sensorId);
        query.bindValue(":val", record.value);
        query.bindValue(":ts", record.timestamp);
        query.exec();
    }
    db.commit();
}

void DBDataControll::onSaveBatchToSql(const QVector<SensorData> &batch) {
    if (batch.isEmpty() || !m_dbInitialized) {
        return;
    }

    saveBatch(batch);
    emit batchCommitted();
    emit sensorStatisticsLoaded(loadSensorStatistics());
}

SensorStatistics DBDataControll::loadSensorStatistics() const {
    SensorStatistics stats;

    if (!m_dbInitialized) {
        return stats;
    }

    const qint64 cutoffMs = QDateTime::currentMSecsSinceEpoch() - SENSOR_ACTIVITY_WINDOW_MS;

    QSqlQuery query(QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)));
    query.prepare(QStringLiteral(
        "SELECT "
        "  COUNT(DISTINCT sensor_id), "
        "  COALESCE(AVG(value), 0), "
        "  COALESCE(MIN(value), 0), "
        "  COALESCE(MAX(value), 0) "
        "FROM telemetry "
        "WHERE timestamp >= :cutoff"));
    query.bindValue(":cutoff", cutoffMs);

    if (!query.exec()) {
        qCritical() << "Ошибка запроса статистики датчиков:" << query.lastError().text();
        return stats;
    }

    if (query.next()) {
        stats.setConnectedCount(query.value(0).toInt());
        stats.setAverageValue(query.value(1).toDouble());
        stats.setMinimumValue(query.value(2).toDouble());
        stats.setMaximumValue(query.value(3).toDouble());
    }

    return stats;
}

QVector<SensorData> DBDataControll::loadSortedWindowWithFilter(const FilterQuerySpec &filterSpec,
                                                               int sortColumn, int sortOrder,
                                                               int limit) const {
    const QString filterSqlCondition = filterSpec.toSqlCondition();
    QVector<SensorData> result;
    if (limit <= 0) {
        return result;
    }

    result.reserve(limit);

    const QString column = sortColumnSql(sortColumn);
    const bool ascending = sortOrder == static_cast<int>(Qt::AscendingOrder);
    const QString direction = ascending ? QStringLiteral("ASC") : QStringLiteral("DESC");

    QSqlQuery query(QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)));
    query.setForwardOnly(true);

    QString queryString = QStringLiteral(
        "SELECT id, sensor_id, value, timestamp FROM telemetry");
    if (!filterSqlCondition.isEmpty()) {
        queryString += QStringLiteral(" WHERE ") + filterSqlCondition;
    }
    queryString += QStringLiteral(" ORDER BY %1 %2, id %2 LIMIT %3")
                       .arg(column, direction)
                       .arg(limit);

    if (query.exec(queryString)) {
        while (query.next()) {
            result.append(readRow(query));
        }
    } else {
        qCritical() << "Ошибка фильтрации в SQL:" << query.lastError().text();
    }

    return result;
}

void DBDataControll::fetchSensorStatistics() {
    emit sensorStatisticsLoaded(loadSensorStatistics());
}

void DBDataControll::fetchSortedWindow(int sortColumn, int sortOrder, int limit) {
    if (!m_dbInitialized) {
        emit dataLoaded({});
        return;
    }
    m_hasActiveFilter = false;

    QVector<SensorData> result;
    result.reserve(limit);

    const QString column = sortColumnSql(sortColumn);
    const bool ascending = sortOrder == static_cast<int>(Qt::AscendingOrder);
    const QString direction = ascending ? QStringLiteral("ASC") : QStringLiteral("DESC");

    QSqlQuery query(QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)));
    query.setForwardOnly(true);

    const QString queryString = QStringLiteral(
        "SELECT id, sensor_id, value, timestamp FROM telemetry "
        "ORDER BY %1 %2, id %2 LIMIT %3").arg(column, direction).arg(limit);

    if (query.exec(queryString)) {
        while (query.next()) {
            result.append(readRow(query));
        }
    } else {
        qCritical() << "Ошибка сортировки в SQL:" << query.lastError().text();
    }

    emit dataLoaded(result);
}

void DBDataControll::fetchSortedTail(int sortColumn, int sortOrder, int limit) {
    if (!m_dbInitialized) {
        emit tailDataLoaded({});
        return;
    }
    m_hasActiveFilter = false;

    QVector<SensorData> result;
    result.reserve(limit);

    const QString column = sortColumnSql(sortColumn);
    const bool ascending = sortOrder == static_cast<int>(Qt::AscendingOrder);
    const QString direction = ascending ? QStringLiteral("DESC") : QStringLiteral("ASC");

    QSqlQuery query(QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)));
    query.setForwardOnly(true);

    const QString queryString = QStringLiteral(
        "SELECT id, sensor_id, value, timestamp FROM telemetry "
        "ORDER BY %1 %2, id %2 LIMIT %3").arg(column, direction).arg(limit);

    if (query.exec(queryString)) {
        while (query.next()) {
            result.append(readRow(query));
        }
    } else {
        qCritical() << "Ошибка загрузки хвоста в SQL:" << query.lastError().text();
    }

    std::reverse(result.begin(), result.end());
    emit tailDataLoaded(result);
}

void DBDataControll::fetchRangeNearAnchor(int sortColumn, int sortOrder,
                                          quint64 anchorRecordId, int limit,
                                          Telemetry::AnchorSide side) {
    if (!m_dbInitialized || anchorRecordId == 0) {
        emit rangeNearAnchorLoaded({}, side);
        return;
    }

    const QString column = sortColumnSql(sortColumn);
    const bool ascending = sortOrder == static_cast<int>(Qt::AscendingOrder);
    const QString activeFilterSql = m_hasActiveFilter
        ? m_activeFilterSpec.toSqlCondition(QStringLiteral("t"))
        : QString();
    emit rangeNearAnchorLoaded(
        loadRangeNearAnchor(anchorRecordId, limit, column, ascending, side, activeFilterSql), side);
}

void DBDataControll::applyFilterQuery(const FilterQuerySpec &filterSpec, int sortColumn,
                                      int sortOrder, int limit) {
    if (!m_dbInitialized) {
        emit dataLoaded({});
        return;
    }

    m_hasActiveFilter = true;
    m_activeFilterSpec = filterSpec;
    emit dataLoaded(loadSortedWindowWithFilter(filterSpec, sortColumn, sortOrder, limit));
}

void DBDataControll::clearDatabase() {
    if (!m_dbInitialized) {
        return;
    }

    QSqlQuery query(QSqlDatabase::database(QLatin1String(DbConnection::SQL_WORKER_CONNECTION_NAME)));
    if (!query.exec("DELETE FROM telemetry")) {
        qCritical() << "Не удалось очистить БД:" << query.lastError().text();
    }
    // Сбрасываем автонумерацию PK, чтобы после очистки id начинался заново.
    if (!query.exec("DELETE FROM sqlite_sequence WHERE name='telemetry'")) {
        qCritical() << "Не удалось сбросить sequence telemetry:" << query.lastError().text();
    }
    m_hasActiveFilter = false;
    emit sensorStatisticsLoaded({});
    emit databaseCleared();
}
