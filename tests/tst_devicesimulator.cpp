#include "tst_devicesimulator.h"

#include "Domain/telemetrytypes.h"
#include "Infrastructure/Devices/devicesimulator.h"
#include "testconstants.h"

#include <QSignalSpy>
#include <QVector>
#include <QtTest>

namespace {

Telemetry::ConnectionStatus connectionStatusFromSpy(const QSignalSpy &spy, int index)
{
    return spy.at(index).at(0).value<Telemetry::ConnectionStatus>();
}

} // namespace

void TestDeviceSimulator::startGeneration_emitsStartedStatus()
{
    DeviceSimulator simulator;
    QSignalSpy statusSpy(&simulator, &DeviceSimulator::connectionStatusChanged);

    simulator.startGeneration();

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(connectionStatusFromSpy(statusSpy, 0), Telemetry::ConnectionStatus::Started);
}

void TestDeviceSimulator::stopGeneration_emitsStoppedStatus()
{
    DeviceSimulator simulator;
    QSignalSpy statusSpy(&simulator, &DeviceSimulator::connectionStatusChanged);

    simulator.startGeneration();
    simulator.stopGeneration();

    QCOMPARE(statusSpy.count(), 2);
    QCOMPARE(connectionStatusFromSpy(statusSpy, 1), Telemetry::ConnectionStatus::Stopped);
}

void TestDeviceSimulator::startGeneration_whenAlreadyStarted_doesNotEmitAgain()
{
    DeviceSimulator simulator;
    QSignalSpy statusSpy(&simulator, &DeviceSimulator::connectionStatusChanged);

    simulator.startGeneration();
    const int countAfterFirstStart = statusSpy.count();
    simulator.startGeneration();

    QCOMPARE(statusSpy.count(), countAfterFirstStart);
    QCOMPARE(connectionStatusFromSpy(statusSpy, 0), Telemetry::ConnectionStatus::Started);
}

void TestDeviceSimulator::stopGeneration_whenNotStarted_emitsStoppedStatus()
{
    DeviceSimulator simulator;
    QSignalSpy statusSpy(&simulator, &DeviceSimulator::connectionStatusChanged);

    simulator.stopGeneration();

    QCOMPARE(statusSpy.count(), 1);
    QCOMPARE(connectionStatusFromSpy(statusSpy, 0), Telemetry::ConnectionStatus::Stopped);
}

void TestDeviceSimulator::startThenStop_emitsStartedThenStopped()
{
    DeviceSimulator simulator;
    QSignalSpy statusSpy(&simulator, &DeviceSimulator::connectionStatusChanged);

    simulator.startGeneration();
    simulator.stopGeneration();

    QCOMPARE(statusSpy.count(), 2);
    QCOMPARE(connectionStatusFromSpy(statusSpy, 0), Telemetry::ConnectionStatus::Started);
    QCOMPARE(connectionStatusFromSpy(statusSpy, 1), Telemetry::ConnectionStatus::Stopped);
}

void TestDeviceSimulator::startGeneration_producesDataBatch()
{
    DeviceSimulator simulator;
    QSignalSpy batchSpy(&simulator, &DeviceSimulator::rawDataGenerated);

    simulator.startGeneration();
    QVERIFY(QTest::qWaitFor([&batchSpy]() { return batchSpy.count() > 0; },
                            TestConstants::DEVICE_SIMULATOR_WAIT_MS));

    const auto batch = batchSpy.at(0).at(0).value<QVector<SensorData>>();
    QCOMPARE(batch.size(), DeviceSimulator::GENERATION_BATCH_SIZE);

    simulator.stopGeneration();
}
