#include "tst_filterqueryspec.h"
#include "Domain/filterqueryspec.h"
#include "testhelpers.h"

#include <QtTest>

void TestFilterQuerySpec::gettersReturnDefaults()
{
    const FilterQuerySpec spec;
    QCOMPARE(spec.field(), FilterQuerySpec::Field::SensorId);
    QCOMPARE(spec.sensorId(), 0);
    QVERIFY(TestHelpers::nearlyEqual(spec.value(), 0.0));
    QVERIFY(TestHelpers::nearlyEqual(spec.tolerance(), 0.0));
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Near);
    QCOMPARE(spec.fromTimestampMs(), qint64(0));
    QCOMPARE(spec.toTimestampMs(), qint64(0));
}

void TestFilterQuerySpec::settersUpdateValues()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setSensorId(7);
    spec.setValue(12.34);
    spec.setTolerance(0.25);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Greater);
    spec.setTimestampRange(1000, 5000);

    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.sensorId(), 7);
    QVERIFY(TestHelpers::nearlyEqual(spec.value(), 12.34));
    QVERIFY(TestHelpers::nearlyEqual(spec.tolerance(), 0.25));
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Greater);
    QCOMPARE(spec.fromTimestampMs(), qint64(1000));
    QCOMPARE(spec.toTimestampMs(), qint64(5000));
}

void TestFilterQuerySpec::setField_skipsDuplicate()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Timestamp);
    spec.setField(FilterQuerySpec::Field::Timestamp);
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Timestamp);
}

void TestFilterQuerySpec::setSensorId_skipsDuplicate()
{
    FilterQuerySpec spec;
    spec.setSensorId(42);
    spec.setSensorId(42);
    QCOMPARE(spec.sensorId(), 42);
}

void TestFilterQuerySpec::setValue_skipsNearDuplicate()
{
    FilterQuerySpec spec;
    spec.setValue(10.0);
    spec.setValue(10.0 + 1e-12);
    QVERIFY(TestHelpers::nearlyEqual(spec.value(), 10.0));
}

void TestFilterQuerySpec::setTolerance_skipsNearDuplicate()
{
    FilterQuerySpec spec;
    spec.setTolerance(0.5);
    spec.setTolerance(0.5 + 1e-12);
    QVERIFY(TestHelpers::nearlyEqual(spec.tolerance(), 0.5));
}

void TestFilterQuerySpec::setValueOperation_skipsDuplicate()
{
    FilterQuerySpec spec;
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Less);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Less);
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Less);
}

void TestFilterQuerySpec::setTimestampRange_ordersBounds()
{
    FilterQuerySpec spec;
    spec.setTimestampRange(5000, 1000);
    QCOMPARE(spec.fromTimestampMs(), qint64(1000));
    QCOMPARE(spec.toTimestampMs(), qint64(5000));
}

void TestFilterQuerySpec::setTimestampRange_skipsDuplicate()
{
    FilterQuerySpec spec;
    spec.setTimestampRange(5000, 1000);
    spec.setTimestampRange(1000, 5000);
    QCOMPARE(spec.fromTimestampMs(), qint64(1000));
    QCOMPARE(spec.toTimestampMs(), qint64(5000));
}

void TestFilterQuerySpec::toSqlCondition_sensorId()
{
    FilterQuerySpec spec;
    spec.setSensorId(42);
    QCOMPARE(spec.toSqlCondition(), QStringLiteral("sensor_id = 42"));
}

void TestFilterQuerySpec::toSqlCondition_valueNear()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setValue(100.0);
    spec.setTolerance(0.5);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Near);
    QCOMPARE(spec.toSqlCondition(), QStringLiteral("ABS(value - 100.00) <= 0.5000"));
}

void TestFilterQuerySpec::toSqlCondition_valueGreater()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setValue(50.0);
    spec.setTolerance(0.1);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Greater);
    QCOMPARE(spec.toSqlCondition(), QStringLiteral("value > 50.1000"));
}

void TestFilterQuerySpec::toSqlCondition_valueLess()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Value);
    spec.setValue(50.0);
    spec.setTolerance(0.1);
    spec.setValueOperation(FilterQuerySpec::ValueOperation::Less);
    QCOMPARE(spec.toSqlCondition(), QStringLiteral("value < 49.9000"));
}

void TestFilterQuerySpec::toSqlCondition_timestamp()
{
    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Timestamp);
    spec.setTimestampRange(1000, 5000);
    QCOMPARE(spec.toSqlCondition(), QStringLiteral("timestamp BETWEEN 1000 AND 5000"));
    QCOMPARE(spec.toSqlCondition(QStringLiteral("t")),
             QStringLiteral("t.timestamp BETWEEN 1000 AND 5000"));
}
