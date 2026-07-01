#ifndef DBCONNECT_H
#define DBCONNECT_H

#include <QObject>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QString>

namespace DbConnection {
inline constexpr const char SQL_WORKER_CONNECTION_NAME[] = "SQL_Worker_Connection";
}

class DatabaseConnectionManager : public QObject {
    Q_OBJECT
public:
    /**
     * Создаёт объект для подключения к БД, но не подключается сразу.
     * @dbPath - путь до базы данных
     * @connectionName - имя соединения
     * @parent - родительский объект
     */
    explicit DatabaseConnectionManager(const QString &dbPath,
                                        const QString &connectionName,
                                        QObject *parent = nullptr);
    ~DatabaseConnectionManager();

    // управление подключением
    bool openConnection();
    void closeConnection();

    // Статусы
    bool isConnected() const;
    QString lastError() const;
    QString connectionName() const;

private:
    bool applyPerformancePragmas();

    QString m_dbPath;
    QString m_connectionName;
    mutable QString m_lastError;
};
#endif // DBCONNECT_H
