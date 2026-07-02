#include "tst_filterqueryspec.h"
#include "Domain/filterqueryspec.h"
#include "testconstants.h"
#include "testhelpers.h"

#include <QtTest>

void TestFilterQuerySpec::gettersReturnDefaults()
{
    const FilterQuerySpec spec;
    QCOMPARE(spec.field(), FilterQuerySpec::Field::SensorId);
    QCOMPARE(spec.sensorId(), FilterQuerySpec::DEFAULT_SENSOR_ID);
    QVERIFY(TestHelpers::nearlyEqual(spec.value(), FilterQuerySpec::DEFAULT_VALUE));
    QVERIFY(TestHelpers::nearlyEqual(spec.tolerance(), FilterQuerySpec::DEFAULT_TOLERANCE));
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Near);
    QCOMPARE(spec.fromTimestampMs(), FilterQuerySpec::DEFAULT_TIMESTAMP_MS);
    QCOMPARE(spec.toTimestampMs(), FilterQuerySpec::DEFAULT_TIMESTAMP_MS);
}

void TestFilterQuerySpec::settersUpdateValues()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setSensorId(TestConstants::FILTER_SPEC_SENSOR_ID);
    spec.setValue(TestConstants::STATS_AVERAGE);
    spec.setTolerance(TestConstants::FILTER_SPEC_TOLERANCE);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Greater);
    spec.setTimestampRange(TestConstants::TIMESTAMP_RANGE_FROM_MS,
                           TestConstants::TIMESTAMP_RANGE_TO_MS);

    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.sensorId(), TestConstants::FILTER_SPEC_SENSOR_ID);
    QVERIFY(TestHelpers::nearlyEqual(spec.value(), TestConstants::STATS_AVERAGE));
    QVERIFY(TestHelpers::nearlyEqual(spec.tolerance(), TestConstants::FILTER_SPEC_TOLERANCE));
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Greater);
    QCOMPARE(spec.fromTimestampMs(), TestConstants::TIMESTAMP_RANGE_FROM_MS);
    QCOMPARE(spec.toTimestampMs(), TestConstants::TIMESTAMP_RANGE_TO_MS);
}

void TestFilterQuerySpec::setTimestampRange_ordersBounds()
{
    FilterQuerySpec spec;
    spec.setTimestampRange(TestConstants::TIMESTAMP_RANGE_TO_MS,
                          TestConstants::TIMESTAMP_RANGE_FROM_MS);
    QCOMPARE(spec.fromTimestampMs(), TestConstants::TIMESTAMP_RANGE_FROM_MS);
    QCOMPARE(spec.toTimestampMs(), TestConstants::TIMESTAMP_RANGE_TO_MS);
}

void TestFilterQuerySpec::toSqlCondition_sensorId()
{
    FilterQuerySpec spec;
    spec.setSensorId(TestConstants::SAMPLE_SENSOR_ID);
    QCOMPARE(spec.toSqlCondition(),
             QStringLiteral("sensor_id = %1").arg(TestConstants::SAMPLE_SENSOR_ID));
}

void TestFilterQuerySpec::toSqlCondition_valueNear()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setValue(TestConstants::NEAR_FILTER_VALUE);
    spec.setTolerance(TestConstants::NEAR_FILTER_TOLERANCE);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Near);
    const QString expected = QStringLiteral("ABS(value - %1) <= %2")
                                 .arg(QString::number(TestConstants::NEAR_FILTER_VALUE, 'f',
                                                      FilterQuerySpec::SENSOR_VALUE_DECIMAL_PLACES),
                                      QString::number(TestConstants::NEAR_FILTER_TOLERANCE, 'f',
                                                      FilterQuerySpec::TOLERANCE_DECIMAL_PLACES));
    QCOMPARE(spec.toSqlCondition(), expected);
}

void TestFilterQuerySpec::toSqlCondition_valueGreater()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setValue(TestConstants::BOUNDARY_FILTER_VALUE);
    spec.setTolerance(TestConstants::BOUNDARY_FILTER_TOLERANCE);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Greater);
    const QString expected = QStringLiteral("value > %1")
                                 .arg(QString::number(TestConstants::BOUNDARY_FILTER_VALUE
                                                          + TestConstants::BOUNDARY_FILTER_TOLERANCE,
                                                      'f',
                                                      FilterQuerySpec::TOLERANCE_DECIMAL_PLACES));
    QCOMPARE(spec.toSqlCondition(), expected);
}

void TestFilterQuerySpec::toSqlCondition_valueLess()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setValue(TestConstants::BOUNDARY_FILTER_VALUE);
    spec.setTolerance(TestConstants::BOUNDARY_FILTER_TOLERANCE);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Less);
    const QString expected = QStringLiteral("value < %1")
                                 .arg(QString::number(TestConstants::BOUNDARY_FILTER_VALUE
                                                          - TestConstants::BOUNDARY_FILTER_TOLERANCE,
                                                      'f',
                                                      FilterQuerySpec::TOLERANCE_DECIMAL_PLACES));
    QCOMPARE(spec.toSqlCondition(), expected);
}

void TestFilterQuerySpec::toSqlCondition_timestamp()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Timestamp);
    spec.setTimestampRange(TestConstants::TIMESTAMP_RANGE_FROM_MS,
                           TestConstants::TIMESTAMP_RANGE_TO_MS);
    QCOMPARE(spec.toSqlCondition(),
             QStringLiteral("timestamp BETWEEN %1 AND %2")
                 .arg(TestConstants::TIMESTAMP_RANGE_FROM_MS)
                 .arg(TestConstants::TIMESTAMP_RANGE_TO_MS));
    QCOMPARE(spec.toSqlCondition(QStringLiteral("t")),
             QStringLiteral("t.timestamp BETWEEN %1 AND %2")
                 .arg(TestConstants::TIMESTAMP_RANGE_FROM_MS)
                 .arg(TestConstants::TIMESTAMP_RANGE_TO_MS));
}
