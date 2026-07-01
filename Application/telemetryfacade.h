#ifndef TELEMETRYFACADE_H
#define TELEMETRYFACADE_H

#include <QObject>

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
    void applyFilter(const QString &filterCondition);

private:
    TelemetryViewModel *telemetryViewModelInstance = nullptr;
    StatisticsViewModel *statisticsViewModelInstance = nullptr;
    FilterViewModel *filterViewModelInstance = nullptr;
    ThreadOrchestrator *orchestrator = nullptr;
};

#endif // TELEMETRYFACADE_H
