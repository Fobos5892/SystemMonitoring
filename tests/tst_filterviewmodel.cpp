#include "tst_filterviewmodel.h"
#include "Domain/filterlimits.h"
#include "Domain/sensorlimits.h"
#include "testconstants.h"
#include "ViewModels/filterviewmodel.h"

#include <QDateTime>
#include <QtTest>

void TestFilterViewModel::normalizeValue_clampsAndTruncates()
{
    QCOMPARE(FilterViewModel::normalizeValue(TestConstants::OVER_MAX_SENSOR_VALUE),
             SensorLimits::MAX_VOLTAGE_VOLTS);
    QCOMPARE(FilterViewModel::normalizeValue(TestConstants::BELOW_MIN_SENSOR_VALUE),
             SensorLimits::MIN_VOLTAGE_VOLTS);
    QCOMPARE(FilterViewModel::normalizeValue(TestConstants::TRUNCATE_VALUE_INPUT),
             TestConstants::TRUNCATE_VALUE_EXPECTED);
}

void TestFilterViewModel::normalizeTolerance_clampsAndTruncates()
{
    QCOMPARE(FilterViewModel::normalizeTolerance(TestConstants::OVER_MAX_TOLERANCE),
             FilterLimits::MAX_TOLERANCE);
    QCOMPARE(FilterViewModel::normalizeTolerance(TestConstants::BELOW_MIN_TOLERANCE),
             FilterLimits::MIN_TOLERANCE);
    QCOMPARE(FilterViewModel::normalizeTolerance(TestConstants::TRUNCATE_TOLERANCE_INPUT),
             TestConstants::TRUNCATE_TOLERANCE_EXPECTED);
}

void TestFilterViewModel::adaptiveToleranceStep_selectsFineStepForSmallTolerance()
{
    QCOMPARE(FilterViewModel::adaptiveToleranceStep(TestConstants::SMALL_TOLERANCE_INPUT),
             FilterLimits::FINE_TOLERANCE_STEP);
    QCOMPARE(FilterViewModel::adaptiveToleranceStep(TestConstants::LARGE_TOLERANCE_INPUT),
             FilterLimits::COARSE_TOLERANCE_STEP);
}

void TestFilterViewModel::buildQuerySpec_sensorId()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::SensorId);
    vm.setSensorId(TestConstants::SAMPLE_SENSOR_ID);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::SensorId);
    QCOMPARE(spec.sensorId(), TestConstants::SAMPLE_SENSOR_ID);
}

void TestFilterViewModel::buildQuerySpec_valueNear()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(TestConstants::NEAR_FILTER_VALUE, TestConstants::NEAR_FILTER_TOLERANCE,
                      FilterViewModel::ValueOperation::Near);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Near);
    QCOMPARE(spec.value(), TestConstants::NEAR_FILTER_VALUE);
    QCOMPARE(spec.tolerance(), TestConstants::NEAR_FILTER_TOLERANCE);
}

void TestFilterViewModel::buildQuerySpec_valueGreater()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(TestConstants::BOUNDARY_FILTER_VALUE,
                      TestConstants::BOUNDARY_FILTER_TOLERANCE,
                      FilterViewModel::ValueOperation::Greater);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Greater);
    QCOMPARE(spec.value(), TestConstants::BOUNDARY_FILTER_VALUE);
    QCOMPARE(spec.tolerance(), TestConstants::BOUNDARY_FILTER_TOLERANCE);
}

void TestFilterViewModel::buildQuerySpec_valueLess()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Value);
    vm.setValueFilter(TestConstants::BOUNDARY_FILTER_VALUE,
                      TestConstants::BOUNDARY_FILTER_TOLERANCE,
                      FilterViewModel::ValueOperation::Less);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Value);
    QCOMPARE(spec.valueOperation(), FilterQuerySpec::ValueOperation::Less);
    QCOMPARE(spec.value(), TestConstants::BOUNDARY_FILTER_VALUE);
    QCOMPARE(spec.tolerance(), TestConstants::BOUNDARY_FILTER_TOLERANCE);
}

void TestFilterViewModel::buildQuerySpec_timestampRange_ordersBounds()
{
    FilterViewModel vm;
    const QDateTime from =
        QDateTime::fromMSecsSinceEpoch(TestConstants::TIMESTAMP_RANGE_FROM_MS);
    const QDateTime to = QDateTime::fromMSecsSinceEpoch(TestConstants::TIMESTAMP_RANGE_TO_MS);
    vm.setField(FilterViewModel::Field::Timestamp);
    vm.setTimestampRange(to, from);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::Timestamp);
    QCOMPARE(spec.fromTimestampMs(), TestConstants::TIMESTAMP_RANGE_FROM_MS);
    QCOMPARE(spec.toTimestampMs(), TestConstants::TIMESTAMP_RANGE_TO_MS);
}

void TestFilterViewModel::combineLocalDateTime_producesValidEpoch()
{
    const QDateTime dateTime = FilterViewModel::combineLocalDateTime(
        QDate(TestConstants::TEST_YEAR, TestConstants::TEST_MONTH, TestConstants::TEST_DAY),
        FilterViewModel::DAY_START_TIME);
    QVERIFY(dateTime.isValid());
    QVERIFY(dateTime.toMSecsSinceEpoch() > FilterQuerySpec::DEFAULT_TIMESTAMP_MS);
}

void TestFilterViewModel::combineLocalDateTime_roundTripsThroughString()
{
    const QDateTime from = FilterViewModel::combineLocalDateTime(
        QDate(TestConstants::TEST_YEAR, TestConstants::TEST_MONTH, TestConstants::TEST_DAY),
        QTime(TestConstants::TEST_FROM_HOUR, TestConstants::TEST_FROM_MINUTE));
    const QDateTime to = FilterViewModel::combineLocalDateTime(
        QDate(TestConstants::TEST_YEAR, TestConstants::TEST_MONTH, TestConstants::TEST_DAY),
        QTime(TestConstants::TEST_TO_HOUR, TestConstants::TEST_TO_MINUTE), true);

    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Timestamp);
    vm.setTimestampRange(from, to);
    const FilterQuerySpec spec = vm.buildQuerySpec();

    QVERIFY(spec.fromTimestampMs() > FilterQuerySpec::DEFAULT_TIMESTAMP_MS);
    QVERIFY(spec.toTimestampMs() > spec.fromTimestampMs());
}

void TestFilterViewModel::buildQuerySpec_timestampOnlyWhenFieldIsTimestamp()
{
    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::SensorId);
    vm.setSensorId(TestConstants::OTHER_SENSOR_ID);
    const FilterQuerySpec spec = vm.buildQuerySpec();
    QCOMPARE(spec.field(), FilterQuerySpec::Field::SensorId);
    QCOMPARE(spec.fromTimestampMs(), FilterQuerySpec::DEFAULT_TIMESTAMP_MS);
    QCOMPARE(spec.toTimestampMs(), FilterQuerySpec::DEFAULT_TIMESTAMP_MS);
}
