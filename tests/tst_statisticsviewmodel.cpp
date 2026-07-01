#include "tst_statisticsviewmodel.h"
#include "ViewModels/statisticsviewmodel.h"

#include <QSignalSpy>
#include <QtTest>

void TestStatisticsViewModel::updateStatistics_formatsLabels()
{
    StatisticsViewModel vm;
    QSignalSpy spy(&vm, &StatisticsViewModel::labelsChanged);

    SensorStatistics stats;
    stats.setConnectedCount(2);
    stats.setAverageValue(10.5);
    stats.setMinimumValue(1.25);
    stats.setMaximumValue(20.0);
    vm.updateStatistics(stats);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(vm.connectedLabel(), QStringLiteral("Активных датчиков: 2"));
    QCOMPARE(vm.averageLabel(), QStringLiteral("Среднее: 10.50 В"));
    QCOMPARE(vm.minimumLabel(), QStringLiteral("Минимум: 1.25 В"));
    QCOMPARE(vm.maximumLabel(), QStringLiteral("Максимум: 20.00 В"));
}

void TestStatisticsViewModel::updateStatistics_skipsDuplicateEmit()
{
    StatisticsViewModel vm;
    SensorStatistics stats;
    stats.setConnectedCount(1);
    stats.setAverageValue(5.0);
    stats.setMinimumValue(5.0);
    stats.setMaximumValue(5.0);

    vm.updateStatistics(stats);
    QSignalSpy spy(&vm, &StatisticsViewModel::labelsChanged);
    vm.updateStatistics(stats);
    QCOMPARE(spy.count(), 0);
}
