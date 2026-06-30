#ifndef DBCONNECT_H
#define DBCONNECT_H

#include <QObject>
#include <QtSql/QSqlError>
#include <QtSql/QSqlDatabase>
#include <QString>

class DatabaseConnectionManager : public QObject {
    Q_OBJECT
public:
    // connectionName позволяет создавать уникальные подключения для разных потоков
    explicit DatabaseConnectionManager(const QString &dbPath,
                                        const QString &connectionName = "DefaultConnection",
                                        QObject *parent = nullptr);
    ~DatabaseConnectionManager();

    // Основные методы управления жизненным циклом подключения
    bool openConnection();
    void closeConnection();

    // Методы проверки статуса
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
