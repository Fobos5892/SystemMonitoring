#include "tst_filterviewmodel.h"
#include "ViewModels/filterviewmodel.h"

#include <QDateTime>
#include <QtTest>

void TestFilterViewModel::normalizeValue_clampsAndTruncates()
{
    QCOMPARE(FilterViewModel::normalizeValue(300.0), 280.0);
    QCOMPARE(FilterViewModel::normalizeValue(-5.0), 0.0);
    QCOMPARE(FilterViewModel::normalizeValue(12.349), 12.34);
}

void TestFilterViewModel::normalizeTolerance_clampsAndTruncates()
{
    QCOMPARE(FilterViewModel::normalizeTolerance(2.0), 1.0);
    QCOMPARE(FilterViewModel::normalizeTolerance(-1.0), 0.0);
    QCOMPARE(FilterViewModel::normalizeTolerance(0.123456), 0.1234);
}

void TestFilterViewModel::adaptiveToleranceStep_selectsFineStepForSmallTolerance()
{
    QCOMPARE(FilterViewModel::adaptiveToleranceStep(0.001), 0.0001);
    QCOMPARE(FilterViewModel::adaptiveToleranceStep(0.05), 0.01);
}

void TestFilterViewModel::buildSqlCondition_sensorId()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::SensorId);
    vm.setSensorId(42);
    QCOMPARE(vm.buildSqlCondition(), QStringLiteral("sensor_id = 42"));
}

void TestFilterViewModel::buildSqlCondition_valueNear()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(100.0, 0.5, FilterViewModel::ValueOperation::Near);
    QCOMPARE(vm.buildSqlCondition(), QStringLiteral("ABS(value - 100.00) <= 0.5000"));
}

void TestFilterViewModel::buildSqlCondition_valueGreater()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(50.0, 0.1, FilterViewModel::ValueOperation::Greater);
    QCOMPARE(vm.buildSqlCondition(), QStringLiteral("value > 50.1000"));
}

void TestFilterViewModel::buildSqlCondition_valueLess()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(50.0, 0.1, FilterViewModel::ValueOperation::Less);
    QCOMPARE(vm.buildSqlCondition(), QStringLiteral("value < 49.9000"));
}

void TestFilterViewModel::buildSqlCondition_timestampRange_ordersBounds()
{
    FilterViewModel vm;
    const QDateTime from = QDateTime::fromMSecsSinceEpoch(1000);
    const QDateTime to = QDateTime::fromMSecsSinceEpoch(5000);
    vm.setField(FilterViewModel::Field::Timestamp);
    vm.setTimestampRange(to, from);

    QCOMPARE(vm.buildSqlCondition(), QStringLiteral("timestamp BETWEEN 1000 AND 5000"));
}
