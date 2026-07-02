#ifndef TELEMETRYFACADE_H
#define TELEMETRYFACADE_H

#include <QObject>
#include <QScopedPointer>
#include "Domain/filterqueryspec.h"

class FilterViewModel;
class StatisticsViewModel;
class TelemetryTableModel;
class TelemetryViewModel;
class ThreadOrchestrator;

class TelemetryFacade : public QObject {
    Q_OBJECT

public:
    explicit TelemetryFacade(QObject *parent = nullptr);
    ~TelemetryFacade() override;

    TelemetryViewModel* telemetryViewModel() const;
    TelemetryTableModel* tableModel() const;
    StatisticsViewModel* statisticsViewModel() const;
    FilterViewModel* filterViewModel() const;

    void start();
    void startGeneration();
    void stopGeneration();
    void clearDatabase();
    void applyFilter(const FilterQuerySpec &filterSpec, int sortColumn, int sortOrder, int limit);

private:
    QScopedPointer<TelemetryViewModel> telemetryViewModelInstance;
    QScopedPointer<StatisticsViewModel> statisticsViewModelInstance;
    QScopedPointer<FilterViewModel> filterViewModelInstance;
    QScopedPointer<ThreadOrchestrator> orchestrator;
};

#endif // TELEMETRYFACADE_H
