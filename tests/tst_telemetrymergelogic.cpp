#include "tst_telemetrymergelogic.h"

#include "Domain/filterqueryspec.h"
#include "Domain/telemetrymergelogic.h"
#include "testconstants.h"
#include "testhelpers.h"

#include <QtTest>

namespace {

SensorData makeRecord(quint64 recordId, int sensorId, double value, quint64 timestamp)
{
    return TestHelpers::makeRecord(recordId, sensorId, value, timestamp);
}

FilterQuerySpec sensorFilter(int sensorId)
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::SensorId);
    spec.setSensorId(sensorId);
    return spec;
}

} // namespace

void TestTelemetryMergeLogic::compareBySort_ordersByTimestampAscending()
{
    const SensorData earlier = makeRecord(1, 1, 10.0, 100);
    const SensorData later = makeRecord(2, 1, 20.0, 200);

    QVERIFY(TelemetryMerge::compareBySort(earlier, later,
                                          static_cast<int>(Telemetry::Column::Timestamp),
                                          Qt::AscendingOrder)
            < 0);
    QVERIFY(TelemetryMerge::compareBySort(later, earlier,
                                          static_cast<int>(Telemetry::Column::Timestamp),
                                          Qt::AscendingOrder)
            > 0);
}

void TestTelemetryMergeLogic::findInsertIndex_insertsIntoMiddle()
{
    QVector<SensorData> records = {
        makeRecord(1, 1, 10.0, 100),
        makeRecord(3, 1, 30.0, 300),
    };
    const SensorData middle = makeRecord(2, 1, 20.0, 200);

    const int index = TelemetryMerge::findInsertIndex(
        records,
        middle,
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder);
    QCOMPARE(index, 1);
}

void TestTelemetryMergeLogic::shouldAcceptAtViewportEdge_middleAcceptsOnlyWithinWindow()
{
    const QVector<SensorData> records = {
        makeRecord(1, 1, 10.0, 100),
        makeRecord(2, 1, 20.0, 200),
    };
    const SensorData inside = makeRecord(3, 1, 15.0, 150);
    const SensorData outside = makeRecord(4, 1, 25.0, 250);

    QVERIFY(TelemetryMerge::shouldAcceptAtViewportEdge(
        inside,
        records,
        Telemetry::ViewportZone::Middle,
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder));
    QVERIFY(!TelemetryMerge::shouldAcceptAtViewportEdge(
        outside,
        records,
        Telemetry::ViewportZone::Middle,
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder));
}

void TestTelemetryMergeLogic::shouldAcceptAtViewportEdge_bottomAcceptsNewerRows()
{
    const QVector<SensorData> records = {
        makeRecord(1, 1, 10.0, 100),
        makeRecord(2, 1, 20.0, 200),
    };
    const SensorData newer = makeRecord(3, 1, 30.0, 300);
    const SensorData older = makeRecord(4, 1, 5.0, 50);

    QVERIFY(TelemetryMerge::shouldAcceptAtViewportEdge(
        newer,
        records,
        Telemetry::ViewportZone::BottomEdge,
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder));
    QVERIFY(!TelemetryMerge::shouldAcceptAtViewportEdge(
        older,
        records,
        Telemetry::ViewportZone::BottomEdge,
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder));
}

void TestTelemetryMergeLogic::mergeFilteredInsertions_skipsWhenScrolling()
{
    QVector<SensorData> records = {makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100)};
    const QVector<SensorData> inserted = {makeRecord(2, TestConstants::SAMPLE_SENSOR_ID, 11.0, 110)};

    const TelemetryMerge::MergeOutcome outcome = TelemetryMerge::mergeFilteredInsertions(
        records,
        inserted,
        sensorFilter(TestConstants::SAMPLE_SENSOR_ID),
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder,
        TestConstants::DEFAULT_QUERY_LIMIT,
        false,
        Telemetry::ViewportZone::BottomEdge);

    QCOMPARE(outcome.insertedCount, 0);
    QVERIFY(!outcome.changed);
    QCOMPARE(records.size(), 1);
}

void TestTelemetryMergeLogic::mergeFilteredInsertions_insertsMatchingRowsAtBottomEdge()
{
    QVector<SensorData> records = {
        makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100),
        makeRecord(2, TestConstants::SAMPLE_SENSOR_ID, 20.0, 200),
    };
    const QVector<SensorData> inserted = {
        makeRecord(3, TestConstants::SAMPLE_SENSOR_ID, 30.0, 300),
        makeRecord(4, TestConstants::OTHER_SENSOR_ID, 40.0, 400),
    };

    const TelemetryMerge::MergeOutcome outcome = TelemetryMerge::mergeFilteredInsertions(
        records,
        inserted,
        sensorFilter(TestConstants::SAMPLE_SENSOR_ID),
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder,
        TestConstants::DEFAULT_QUERY_LIMIT,
        true,
        Telemetry::ViewportZone::BottomEdge);

    QCOMPARE(outcome.insertedCount, 1);
    QVERIFY(outcome.changed);
    QCOMPARE(records.size(), 3);
    QCOMPARE(records.last().recordId, static_cast<quint64>(3));
}

void TestTelemetryMergeLogic::mergeFilteredInsertions_insertsIntoMiddleWindow()
{
    QVector<SensorData> records = {
        makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100),
        makeRecord(3, TestConstants::SAMPLE_SENSOR_ID, 30.0, 300),
    };
    const QVector<SensorData> inserted = {makeRecord(2, TestConstants::SAMPLE_SENSOR_ID, 20.0, 200)};

    const TelemetryMerge::MergeOutcome outcome = TelemetryMerge::mergeFilteredInsertions(
        records,
        inserted,
        sensorFilter(TestConstants::SAMPLE_SENSOR_ID),
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder,
        TestConstants::DEFAULT_QUERY_LIMIT,
        true,
        Telemetry::ViewportZone::Middle);

    QCOMPARE(outcome.insertedCount, 1);
    QCOMPARE(records.size(), 3);
    QCOMPARE(records.at(1).recordId, static_cast<quint64>(2));
}

void TestTelemetryMergeLogic::mergeFilteredInsertions_skipsDuplicates()
{
    QVector<SensorData> records = {makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100)};
    const QVector<SensorData> inserted = {makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100)};

    const TelemetryMerge::MergeOutcome outcome = TelemetryMerge::mergeFilteredInsertions(
        records,
        inserted,
        sensorFilter(TestConstants::SAMPLE_SENSOR_ID),
        static_cast<int>(Telemetry::Column::Timestamp),
        Qt::AscendingOrder,
        TestConstants::DEFAULT_QUERY_LIMIT,
        true,
        Telemetry::ViewportZone::BottomEdge);

    QCOMPARE(outcome.insertedCount, 0);
    QCOMPARE(records.size(), 1);
}
