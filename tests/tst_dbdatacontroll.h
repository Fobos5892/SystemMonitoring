#ifndef TST_DBDATACONTROLL_H
#define TST_DBDATACONTROLL_H

#include <QObject>

class TestDBDataControll : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void initializeDatabase_createsPersistentSchema();
    void saveAndFetch_sortedByTimestamp();
    void clearDatabase_removesRows();
    void fetchSensorStatistics_aggregatesSavedBatch();

private:
    class QTemporaryDir *tempDir = nullptr;
    class DBDataControll *controller = nullptr;
};

#endif // TST_DBDATACONTROLL_H
