#include "tst_sensorstatistics.h"
#include "Domain/sensorstatistics.h"
#include "testconstants.h"

#include <QtTest>

void TestSensorStatistics::gettersReturnDefaults()
{
    const SensorStatistics stats;
    QCOMPARE(stats.connectedCount(), SensorStatistics::DEFAULT_CONNECTED_COUNT);
    QCOMPARE(stats.averageValue(), SensorStatistics::DEFAULT_AVERAGE_VALUE);
    QCOMPARE(stats.minimumValue(), SensorStatistics::DEFAULT_MINIMUM_VALUE);
    QCOMPARE(stats.maximumValue(), SensorStatistics::DEFAULT_MAXIMUM_VALUE);
}

void TestSensorStatistics::settersUpdateValues()
{
    SensorStatistics stats;
    stats.setConnectedCount(TestConstants::STATS_CONNECTED_COUNT);
    stats.setAverageValue(TestConstants::STATS_AVERAGE);
    stats.setMinimumValue(TestConstants::STATS_MINIMUM);
    stats.setMaximumValue(TestConstants::STATS_MAXIMUM);

    QCOMPARE(stats.connectedCount(), TestConstants::STATS_CONNECTED_COUNT);
    QCOMPARE(stats.averageValue(), TestConstants::STATS_AVERAGE);
    QCOMPARE(stats.minimumValue(), TestConstants::STATS_MINIMUM);
    QCOMPARE(stats.maximumValue(), TestConstants::STATS_MAXIMUM);
}
