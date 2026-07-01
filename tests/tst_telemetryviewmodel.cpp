#include "tst_telemetryviewmodel.h"
#include "ViewModels/telemetryviewmodel.h"

#include <QSignalSpy>
#include <QtTest>

namespace {

SensorData makeRecord(quint64 recordId, uint64_t sensorId = 1, double value = 10.0, uint64_t timestamp = 1000)
{
    return {recordId, sensorId, timestamp, value};
}

} // namespace

void TestTelemetryViewModel::beginReloading_clearsRecordsAndSetsReloading()
{
    TelemetryViewModel vm;
    QSignalSpy loadingSpy(&vm, &TelemetryViewModel::loadingStarted);

    vm.onDataLoaded({makeRecord(1)});
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

    const QVector<SensorData> chunk = {makeRecord(1), makeRecord(2)};
    vm.onDataLoaded(chunk);

    QCOMPARE(vm.recordCount(), 2);
    QCOMPARE(vm.recordAt(0).recordId, 1u);
    QCOMPARE(vm.recordAt(1).recordId, 2u);
    QCOMPARE(liveSpy.count(), 1);
}

void TestTelemetryViewModel::onDataLoaded_appendsNewRecords()
{
    TelemetryViewModel vm;
    vm.onDataLoaded({makeRecord(1), makeRecord(2)});

    QSignalSpy liveSpy(&vm, &TelemetryViewModel::liveDataInserted);
    vm.onDataLoaded({makeRecord(1), makeRecord(2), makeRecord(3)});

    QCOMPARE(vm.recordCount(), 3);
    QCOMPARE(vm.recordAt(2).recordId, 3u);
    QCOMPARE(liveSpy.count(), 1);
}

void TestTelemetryViewModel::onDataLoaded_skipsSameVisibleRange()
{
    TelemetryViewModel vm;
    const QVector<SensorData> chunk = {makeRecord(10), makeRecord(11)};
    vm.onDataLoaded(chunk);

    QSignalSpy liveSpy(&vm, &TelemetryViewModel::liveDataInserted);
    vm.onDataLoaded(chunk);
    QCOMPARE(vm.recordCount(), 2);
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
    vm.onDataLoaded({makeRecord(1)});
    QSignalSpy finishedSpy(&vm, &TelemetryViewModel::loadingFinished);

    vm.onDatabaseCleared();

    QCOMPARE(vm.recordCount(), 0);
    QVERIFY(!vm.isReloading());
    QCOMPARE(finishedSpy.count(), 1);
}
