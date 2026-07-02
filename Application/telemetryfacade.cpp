#include "telemetryfacade.h"

#include "Application/Coordination/threadorchestrator.h"
#include "ViewModels/filterviewmodel.h"
#include "ViewModels/statisticsviewmodel.h"
#include "ViewModels/telemetryviewmodel.h"

TelemetryFacade::TelemetryFacade(QObject *parent)
    : QObject(parent)
    , telemetryViewModelInstance(new TelemetryViewModel())
    , statisticsViewModelInstance(new StatisticsViewModel())
    , filterViewModelInstance(new FilterViewModel())
    , orchestrator(new ThreadOrchestrator(telemetryViewModelInstance.data()))
{
    connect(orchestrator.data(), &ThreadOrchestrator::sensorStatisticsUpdated,
            statisticsViewModelInstance.data(), &StatisticsViewModel::updateStatistics);
}

TelemetryFacade::~TelemetryFacade() = default;

TelemetryViewModel* TelemetryFacade::telemetryViewModel() const
{
    return telemetryViewModelInstance.data();
}

TelemetryTableModel* TelemetryFacade::tableModel() const
{
    return telemetryViewModelInstance->tableModel();
}

StatisticsViewModel* TelemetryFacade::statisticsViewModel() const
{
    return statisticsViewModelInstance.data();
}

FilterViewModel* TelemetryFacade::filterViewModel() const
{
    return filterViewModelInstance.data();
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

void TelemetryFacade::applyFilter(const FilterQuerySpec &filterSpec, int sortColumn,
                                  int sortOrder, int limit)
{
    orchestrator->onFilterRequested(filterSpec, sortColumn, sortOrder, limit);
}
