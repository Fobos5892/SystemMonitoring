#ifndef THREADORCHESTRATOR_H
#define THREADORCHESTRATOR_H

#include <QObject>
#include <QPointer>
#include <QThread>
#include <QScopedPointer>
#include <QString>
#include <QTimer>
#include "Domain/filterqueryspec.h"
#include "Domain/sensorstatistics.h"
#include "Domain/telemetrytypes.h"

class DeviceSimulator;
class DeviceReceiver;
class DbTelemetryRepository;
class ITelemetryRepository;
class TelemetryViewModel;

class ThreadOrchestrator : public QObject {
    Q_OBJECT
public:
    explicit ThreadOrchestrator(TelemetryViewModel *viewModel, QObject *parent = nullptr);
    ~ThreadOrchestrator();

    void startAll();
    void stopAll();

signals:
    void sensorStatisticsUpdated(const SensorStatistics &stats);

public slots:
    void onConnectRequested();
    void onStopGenerationRequested();
    void onClearDatabaseRequested();
    void onFilterRequested(const FilterQuerySpec &filterSpec, int sortColumn, int sortOrder, int limit);
    void onBatchCommittedFromDb();
    void onDebouncedFilterRefresh();
    void onSortRequested(int column, int sortOrder);
    void onTailRequest(int sortColumn, int sortOrder, int limit);
    void onRangeNearAnchorRequested(int sortColumn, int sortOrder, quint64 anchorRecordId,
                                    int limit, Telemetry::AnchorSide side);

private:
    void initThreads();
    void setupConnections();
    void setupSensorConnections();
    void setupRepositoryOutputConnections();
    void setupViewModelQueryConnections();

    QPointer<TelemetryViewModel> viewModel;
    QPointer<ITelemetryRepository> repository;

    QScopedPointer<DeviceSimulator> simulator;
    QScopedPointer<DeviceReceiver> receiver;
    QScopedPointer<DbTelemetryRepository> repositoryAdapter;

    QThread simulatorThread;
    QThread receiverThread;
    QThread sqlThread;

    bool filterActive = false;
    int currentSortColumn = static_cast<int>(Telemetry::Column::Timestamp);
    int currentSortOrder = static_cast<int>(Qt::AscendingOrder);
    int filterLimit = Telemetry::WINDOW_SIZE;
    FilterQuerySpec filterSpec;
    QTimer filterRefreshTimer;
};
#endif // THREADORCHESTRATOR_H
