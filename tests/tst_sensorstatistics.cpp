#include "tst_sensorstatistics.h"
#include "Domain/sensorstatistics.h"

#include <QtTest>

void TestSensorStatistics::gettersReturnDefaults()
{
    const SensorStatistics stats;
    QCOMPARE(stats.connectedCount(), 0);
    QCOMPARE(stats.averageValue(), 0.0);
    QCOMPARE(stats.minimumValue(), 0.0);
    QCOMPARE(stats.maximumValue(), 0.0);
}

void TestSensorStatistics::settersUpdateValues()
{
    SensorStatistics stats;
    stats.setConnectedCount(3);
    stats.setAverageValue(12.34);
    stats.setMinimumValue(1.0);
    stats.setMaximumValue(99.9);

    QCOMPARE(stats.connectedCount(), 3);
    QCOMPARE(stats.averageValue(), 12.34);
    QCOMPARE(stats.minimumValue(), 1.0);
    QCOMPARE(stats.maximumValue(), 99.9);
}

void TestSensorStatistics::setConnectedCount_skipsDuplicate()
{
    SensorStatistics stats;
    stats.setConnectedCount(5);
    stats.setConnectedCount(5);
    QCOMPARE(stats.connectedCount(), 5);
}

void TestSensorStatistics::setDoubleFields_skipsNearDuplicate()
{
    SensorStatistics stats;
    stats.setAverageValue(10.0);
    stats.setAverageValue(10.0 + 1e-12);
    QCOMPARE(stats.averageValue(), 10.0);
}
