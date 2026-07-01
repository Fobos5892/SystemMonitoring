#include "telemetryfacade.h"

#include "Application/Coordination/threadorchestrator.h"
#include "ViewModels/filterviewmodel.h"
#include "ViewModels/statisticsviewmodel.h"
#include "ViewModels/telemetryviewmodel.h"

TelemetryFacade::TelemetryFacade(QObject *parent)
    : QObject(parent)
    , telemetryViewModelInstance(new TelemetryViewModel(this))
    , statisticsViewModelInstance(new StatisticsViewModel(this))
    , filterViewModelInstance(new FilterViewModel(this))
    , orchestrator(new ThreadOrchestrator(telemetryViewModelInstance, this))
{
    connect(orchestrator, &ThreadOrchestrator::sensorStatisticsUpdated,
            statisticsViewModelInstance, &StatisticsViewModel::updateStatistics);
}

TelemetryFacade::~TelemetryFacade() = default;

TelemetryViewModel* TelemetryFacade::telemetryViewModel() const
{
    return telemetryViewModelInstance;
}

TelemetryTableModel* TelemetryFacade::tableModel() const
{
    return telemetryViewModelInstance->tableModel();
}

StatisticsViewModel* TelemetryFacade::statisticsViewModel() const
{
    return statisticsViewModelInstance;
}

FilterViewModel* TelemetryFacade::filterViewModel() const
{
    return filterViewModelInstance;
}

void TelemetryFacade::start()
{
    orchestrator->startAll();
}

void TelemetryFacade::startGeneration()
{
    orchestrator->onConnectRequested();
}

void TelemetryFacade::stopGeneration()
{
    orchestrator->onStopGenerationRequested();
}

void TelemetryFacade::clearDatabase()
{
    orchestrator->onClearDatabaseRequested();
}

void TelemetryFacade::applyFilter(const QString &filterCondition)
{
    orchestrator->onFilterRequested(filterCondition);
}
