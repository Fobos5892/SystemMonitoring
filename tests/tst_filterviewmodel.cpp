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

void TestFilterViewModel::buildQuerySpec_sensorId()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::SensorId);
    vm.setSensorId(42);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::SensorId);
    QCOMPARE(spec.sensorId(), 42);
}

void TestFilterViewModel::buildQuerySpec_valueNear()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(100.0, 0.5, FilterViewModel::ValueOperation::Near);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Near);
    QCOMPARE(spec.value(), 100.0);
    QCOMPARE(spec.tolerance(), 0.5);
}

void TestFilterViewModel::buildQuerySpec_valueGreater()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(50.0, 0.1, FilterViewModel::ValueOperation::Greater);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Greater);
    QCOMPARE(spec.value(), 50.0);
    QCOMPARE(spec.tolerance(), 0.1);
}

void TestFilterViewModel::buildQuerySpec_valueLess()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(50.0, 0.1, FilterViewModel::ValueOperation::Less);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Less);
    QCOMPARE(spec.value(), 50.0);
    QCOMPARE(spec.tolerance(), 0.1);
}

void TestFilterViewModel::buildQuerySpec_timestampRange_ordersBounds()
{
    FilterViewModel vm;
    const QDateTime from = QDateTime::fromMSecsSinceEpoch(1000);
    const QDateTime to = QDateTime::fromMSecsSinceEpoch(5000);
    vm.setField(FilterViewModel::Field::Timestamp);
    vm.setTimestampRange(to, from);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Timestamp);
    QCOMPARE(spec.fromTimestampMs(), 1000);
    QCOMPARE(spec.toTimestampMs(), 5000);
}

void TestFilterViewModel::combineLocalDateTime_producesValidEpoch()
{
    const QDateTime dateTime =
        FilterViewModel::combineLocalDateTime(QDate(2026, 7, 2), QTime(0, 0));
    QVERIFY(dateTime.isValid());
    QVERIFY(dateTime.toMSecsSinceEpoch() > 0);
}

void TestFilterViewModel::buildQuerySpec_timestampOnlyWhenFieldIsTimestamp()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::SensorId);
    vm.setSensorId(5);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::SensorId);
    QCOMPARE(spec.fromTimestampMs(), qint64(0));
    QCOMPARE(spec.toTimestampMs(), qint64(0));
}
