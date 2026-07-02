#ifndef TST_DBDATACONTROLL_H
#define TST_DBDATACONTROLL_H

#include "Infrastructure/Persistence/dbdatacontroll.h"

#include <QObject>
#include <QScopedPointer>
#include <QTemporaryDir>

class TestDBDataControll : public QObject {
    Q_OBJECT

private slots:
    void init();
    void cleanup();

    void initializeDatabase_createsPersistentSchema();
    void saveAndFetch_sortedByTimestamp();
    void clearDatabase_removesRows();
    void fetchSensorStatistics_aggregatesSavedBatch();
    void applyFilterQuery_timestampRange_returnsMatchingRows();
    void onSaveBatchToSql_emitsBatchCommittedWithRecordIds();

private:
    QScopedPointer<QTemporaryDir> tempDir;
    QScopedPointer<DBDataControll> controller;
};

#endif // TST_DBDATACONTROLL_H
