#ifndef THREADORCHESTRATOR_H
#define THREADORCHESTRATOR_H

#include <QObject>
#include <QThread>
#include <QScopedPointer>
#include <QString>
#include <QTimer>

class DeviceSimulator;
class DeviceReceiver;
class DBDataControll;
class SensorModel;

class ThreadOrchestrator : public QObject {
    Q_OBJECT
public:
    explicit ThreadOrchestrator(SensorModel* uiModel, QObject *parent = nullptr);
    ~ThreadOrchestrator();

    void startAll();
    void stopAll();

public slots:
    void onConnectRequested();
    void onStopGenerationRequested();
    void onClearDatabaseRequested();
    void onFilterRequested(const QString &filterCondition);
    void onBatchCommittedFromDb();
    void onDebouncedFilterRefresh();
    void onSortRequested(int column, int sortOrder);
    void onTailWindowRequested(int sortColumn, int sortOrder, int limit);
    void onRangeAfterAnchorRequested(int sortColumn, int sortOrder, quint64 anchorRecordId, int limit);
    void onRangeBeforeAnchorRequested(int sortColumn, int sortOrder, quint64 anchorRecordId, int limit);

private:
    void initThreads();
    void setupConnections();

    SensorModel* m_uiModel;

    QScopedPointer<DeviceSimulator> m_simulator;
    QScopedPointer<DeviceReceiver>  m_receiver;
    QScopedPointer<DBDataControll>  m_dbController;

    QThread m_simulatorThread;
    QThread m_receiverThread;
    QThread m_sqlThread;

    bool m_filterActive = false;
    QString m_filterCondition;
    QTimer m_filterRefreshTimer;
};
#endif // THREADORCHESTRATOR_H
