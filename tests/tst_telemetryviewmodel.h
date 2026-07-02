#ifndef TST_TELEMETRYVIEWMODEL_H
#define TST_TELEMETRYVIEWMODEL_H

#include <QObject>

class TestTelemetryViewModel : public QObject {
    Q_OBJECT

private slots:
    void beginReloading_clearsRecordsAndSetsReloading();
    void onDataLoaded_populatesInitialChunk();
    void onDataLoaded_appendsNewRecords();
    void onDataLoaded_skipsSameVisibleRange();
    void requestSort_emitsSortRequested();
    void setFollowLiveTail_isIdempotent();
    void onDatabaseCleared_resetsState();
    void onBatchCommitted_inFilterMode_mergesWhenScrollIdle();
    void onBatchCommitted_inFilterMode_skipsWhenScrolling();
    void onBatchCommitted_inFilterMode_emitsMergeSignals();
};

#endif // TST_TELEMETRYVIEWMODEL_H
