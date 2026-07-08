#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include <QFileInfo>
#include "dbconnect.h"

DatabaseConnectionManager::DatabaseConnectionManager(const QString &dbPath,
                                                     const QString &connectionName,
                                                     QObject *parent)
    : QObject(parent)
    , m_dbPath(dbPath)
    , m_connectionName(connectionName)
{
}

DatabaseConnectionManager::~DatabaseConnectionManager() {
    closeConnection();
}

bool DatabaseConnectionManager::openConnection() {
    m_lastError.clear();

    // Проверяем, не открыто ли уже соединение с таким именем
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase db = QSqlDatabase::database(m_connectionName);
        if (db.isOpen()) {
            return true;
        }
    }

    // Создаем подключение к драйверу SQLite
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(m_dbPath);

    if (!db.open()) {
        m_lastError = db.lastError().text();
        qCritical() << "Ошибка открытия БД [" << m_connectionName << "]:" << m_lastError;
        return false;
    }

    // Настраиваем оптимизации SQLite (WAL режим, кэш в ОЗУ)
    if (!applyPerformancePragmas()) {
        closeConnection();
        return false;
    }

    qDebug() << "Успешное подключение к БД:" << QFileInfo(m_dbPath).absoluteFilePath()
             << "| Имя соединения:" << m_connectionName;
    return true;
}

void DatabaseConnectionManager::closeConnection() {
    if (QSqlDatabase::contains(m_connectionName)) {
        {
            // Блок {} нужен, чтобы закрыть область видимости db до вызова removeDatabase
            QSqlDatabase db = QSqlDatabase::database(m_connectionName);
            if (db.isOpen()) {
                db.close();
            }
        }
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DatabaseConnectionManager::isConnected() const {
    if (!QSqlDatabase::contains(m_connectionName)) {
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);

    // Не просто проверяем флаг isOpen(), но и делаем легковесный проверочный запрос в СУБД
    if (!db.isOpen()) return false;

    QSqlQuery query(db);
    bool check = query.exec("SELECT 1;");
    if (!check) {
        m_lastError = query.lastError().text();
    }
    return check;
}

QString DatabaseConnectionManager::lastError() const {
    return m_lastError;
}

QString DatabaseConnectionManager::connectionName() const {
    return m_connectionName;
}

bool DatabaseConnectionManager::applyPerformancePragmas() {
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(db);

    const QStringList pragmas = {
        "PRAGMA journal_mode = WAL;", // Сверхбыстрый конкурентный режим записи без блокировки чтения
        "PRAGMA synchronous = OFF;",  // Асинхронная запись на диск
        QStringLiteral("PRAGMA cache_size = %1;").arg(DbConnection::SQLITE_PAGE_CACHE_KIB), // ~200 МБ под кэш страниц
        "PRAGMA temp_store = MEMORY;" // Временные файлы и таблицы только в ОЗУ
    };

    for (const QString &pragma : pragmas) {
        if (query.exec(pragma)) {
            continue;
        }

        m_lastError = query.lastError().text();
        qCritical() << "Ошибка применения PRAGMA настроек скорости:" << m_lastError
                    << "| PRAGMA:" << pragma;
        return false;
    }

    return true;
}