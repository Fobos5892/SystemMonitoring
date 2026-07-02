#include "tst_telemetryviewmodel.h"
#include "Domain/telemetrytypes.h"
#include "testconstants.h"
#include "testhelpers.h"
#include "ViewModels/telemetryviewmodel.h"

#include <QSignalSpy>
#include <QtTest>

void TestTelemetryViewModel::beginReloading_clearsRecordsAndSetsReloading()
{
    TelemetryViewModel vm;
    QSignalSpy loadingSpy(&vm, &TelemetryViewModel::loadingStarted);

    vm.onDataLoaded({TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_FIRST)});
    QCOMPARE(vm.recordCount(), 1);

    vm.beginReloading();
    QCOMPARE(vm.recordCount(), 0);
    QVERIFY(vm.isReloading());
    QCOMPARE(loadingSpy.count(), 1);
}

void TestTelemetryViewModel::onDataLoaded_populatesInitialChunk()
{
    TelemetryViewModel vm;
    QSignalSpy liveSpy(&vm, &TelemetryViewModel::liveDataInserted);

    const QVector<SensorData> chunk = {TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_FIRST),
                                       TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_SECOND)};
    vm.onDataLoaded(chunk);

    QCOMPARE(vm.recordCount(), TestConstants::DB_SENSOR_ID_SECOND);
    QCOMPARE(vm.recordAt(0).recordId, TestConstants::TELEMETRY_RECORD_FIRST);
    QCOMPARE(vm.recordAt(1).recordId, TestConstants::TELEMETRY_RECORD_SECOND);
    QCOMPARE(liveSpy.count(), 1);
}

void TestTelemetryViewModel::onDataLoaded_appendsNewRecords()
{
    TelemetryViewModel vm;
    vm.onDataLoaded({TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_FIRST),
                     TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_SECOND)});

    QSignalSpy liveSpy(&vm, &TelemetryViewModel::liveDataInserted);
    vm.onDataLoaded({TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_FIRST),
                     TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_SECOND),
                     TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_THIRD)});

    QCOMPARE(vm.recordCount(), TestConstants::DB_SENSOR_ID_THIRD);
    QCOMPARE(vm.recordAt(2).recordId, TestConstants::TELEMETRY_RECORD_THIRD);
    QCOMPARE(liveSpy.count(), 1);
}

void TestTelemetryViewModel::onDataLoaded_skipsSameVisibleRange()
{
    TelemetryViewModel vm;
    const QVector<SensorData> chunk = {TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_TEN),
                                       TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_ELEVEN)};
    vm.onDataLoaded(chunk);

    QSignalSpy liveSpy(&vm, &TelemetryViewModel::liveDataInserted);
    vm.onDataLoaded(chunk);
    QCOMPARE(vm.recordCount(), TestConstants::DB_SENSOR_ID_SECOND);
    QCOMPARE(liveSpy.count(), 0);
}

void TestTelemetryViewModel::requestSort_emitsSortRequested()
{
    TelemetryViewModel vm;
    QSignalSpy sortSpy(&vm, &TelemetryViewModel::sortRequested);

    vm.requestSort(static_cast<int>(Telemetry::Column::Value), Qt::DescendingOrder);

    QCOMPARE(sortSpy.count(), 1);
    QCOMPARE(sortSpy.at(0).at(0).toInt(), static_cast<int>(Telemetry::Column::Value));
    QCOMPARE(sortSpy.at(0).at(1).toInt(), static_cast<int>(Qt::DescendingOrder));
    QVERIFY(vm.isReloading());
}

void TestTelemetryViewModel::setFollowLiveTail_isIdempotent()
{
    TelemetryViewModel vm;
    vm.setFollowLiveTail(true);
    vm.setFollowLiveTail(true);
    QVERIFY(vm.isFollowingLiveTail());
}

void TestTelemetryViewModel::onDatabaseCleared_resetsState()
{
    TelemetryViewModel vm;
    vm.onDataLoaded({TestHelpers::makeRecord(TestConstants::TELEMETRY_RECORD_FIRST)});
    QSignalSpy finishedSpy(&vm, &TelemetryViewModel::loadingFinished);

    vm.onDatabaseCleared();

    QCOMPARE(vm.recordCount(), 0);
    QVERIFY(!vm.isReloading());
    QCOMPARE(finishedSpy.count(), 1);
}

namespace {

FilterQuerySpec sensorFilter(int sensorId)
{
    FilterQuerySpec spec;
    spec.setSensorId(sensorId);
    return spec;
}

void loadFilterWindow(TelemetryViewModel &vm, const QVector<SensorData> &chunk)
{
    vm.setFilterMode(true);
    vm.setLiveUpdatesEnabled(false);
    vm.setActiveFilterSpec(sensorFilter(TestConstants::SAMPLE_SENSOR_ID));
    vm.setViewportZone(Telemetry::ViewportZone::BottomEdge);
    vm.beginReloading(TestConstants::DEFAULT_QUERY_LIMIT);
    vm.onDataLoaded(chunk);
}

} // namespace

void TestTelemetryViewModel::onBatchCommitted_inFilterMode_mergesWhenScrollIdle()
{
    TelemetryViewModel vm;
    loadFilterWindow(vm, {
        TestHelpers::makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100),
        TestHelpers::makeRecord(2, TestConstants::SAMPLE_SENSOR_ID, 20.0, 200),
    });
    vm.setScrollIdle(true);

    QSignalSpy liveSpy(&vm, &TelemetryViewModel::liveDataInserted);
    vm.onBatchCommitted({TestHelpers::makeRecord(3, TestConstants::SAMPLE_SENSOR_ID, 30.0, 300)});

    QCOMPARE(vm.recordCount(), 3);
    QCOMPARE(vm.recordAt(2).recordId, static_cast<quint64>(3));
    QCOMPARE(liveSpy.count(), 1);
}

void TestTelemetryViewModel::onBatchCommitted_inFilterMode_skipsWhenScrolling()
{
    TelemetryViewModel vm;
    loadFilterWindow(vm, {
        TestHelpers::makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100),
        TestHelpers::makeRecord(2, TestConstants::SAMPLE_SENSOR_ID, 20.0, 200),
    });
    vm.setScrollIdle(false);

    QSignalSpy liveSpy(&vm, &TelemetryViewModel::liveDataInserted);
    vm.onBatchCommitted({TestHelpers::makeRecord(3, TestConstants::SAMPLE_SENSOR_ID, 30.0, 300)});

    QCOMPARE(vm.recordCount(), 2);
    QCOMPARE(liveSpy.count(), 0);
}

void TestTelemetryViewModel::onBatchCommitted_inFilterMode_emitsMergeSignals()
{
    TelemetryViewModel vm;
    loadFilterWindow(vm, {
        TestHelpers::makeRecord(1, TestConstants::SAMPLE_SENSOR_ID, 10.0, 100),
    });
    vm.setScrollIdle(true);

    QSignalSpy mergeStartSpy(&vm, &TelemetryViewModel::mergeStarted);
    QSignalSpy mergeFinishSpy(&vm, &TelemetryViewModel::mergeFinished);

    vm.onBatchCommitted({TestHelpers::makeRecord(2, TestConstants::SAMPLE_SENSOR_ID, 20.0, 200)});

    QCOMPARE(mergeStartSpy.count(), 1);
    QCOMPARE(mergeFinishSpy.count(), 1);
}
