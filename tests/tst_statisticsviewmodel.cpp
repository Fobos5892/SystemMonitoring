#include "tst_statisticsviewmodel.h"
#include "Domain/telemetrytypes.h"
#include "testconstants.h"
#include "ViewModels/statisticsviewmodel.h"

#include <QSignalSpy>
#include <QtTest>

void TestStatisticsViewModel::updateStatistics_formatsLabels()
{
    StatisticsViewModel vm;
    QSignalSpy spy(&vm, &StatisticsViewModel::labelsChanged);

    SensorStatistics stats;
    stats.setConnectedCount(TestConstants::STATS_VM_CONNECTED);
    stats.setAverageValue(TestConstants::STATS_VM_AVERAGE);
    stats.setMinimumValue(TestConstants::STATS_VM_MINIMUM);
    stats.setMaximumValue(TestConstants::STATS_VM_MAXIMUM);
    vm.updateStatistics(stats);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(vm.connectedLabel(),
             QStringLiteral("Активных датчиков(за %1 минут): %2")
                 .arg(Telemetry::SENSOR_ACTIVITY_WINDOW_MINUTES)
                 .arg(TestConstants::STATS_VM_CONNECTED));
    QCOMPARE(vm.averageLabel(),
             QStringLiteral("Среднее: %1 В")
                 .arg(QString::number(TestConstants::STATS_VM_AVERAGE, 'f',
                                      Telemetry::VOLTAGE_DISPLAY_DECIMAL_PLACES)));
    QCOMPARE(vm.minimumLabel(),
             QStringLiteral("Минимум: %1 В")
                 .arg(QString::number(TestConstants::STATS_VM_MINIMUM, 'f',
                                      Telemetry::VOLTAGE_DISPLAY_DECIMAL_PLACES)));
    QCOMPARE(vm.maximumLabel(),
             QStringLiteral("Максимум: %1 В")
                 .arg(QString::number(TestConstants::STATS_VM_MAXIMUM, 'f',
                                      Telemetry::VOLTAGE_DISPLAY_DECIMAL_PLACES)));
}

void TestStatisticsViewModel::updateStatistics_skipsDuplicateEmit()
{
    StatisticsViewModel vm;
    SensorStatistics stats;
    stats.setConnectedCount(TestConstants::STATS_VM_UNIFORM_CONNECTED);
    stats.setAverageValue(TestConstants::STATS_VM_UNIFORM);
    stats.setMinimumValue(TestConstants::STATS_VM_UNIFORM);
    stats.setMaximumValue(TestConstants::STATS_VM_UNIFORM);

    vm.updateStatistics(stats);
    QSignalSpy spy(&vm, &StatisticsViewModel::labelsChanged);
    vm.updateStatistics(stats);
    QCOMPARE(spy.count(), 0);
}
