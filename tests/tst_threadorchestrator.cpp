#include "tst_threadorchestrator.h"

#include "Application/Coordination/threadorchestrator.h"
#include "Domain/telemetrytypes.h"
#include "ViewModels/telemetryviewmodel.h"
#include "testconstants.h"

#include <QSignalSpy>
#include <QtTest>

namespace {

Telemetry::ConnectionStatus connectionStatusFromSpy(const QSignalSpy &spy, int index)
{
    return spy.at(index).at(0).value<Telemetry::ConnectionStatus>();
}

} // namespace

void TestThreadOrchestrator::forwardsConnectionStatusFromSimulator()
{
    TelemetryViewModel viewModel;
    ThreadOrchestrator orchestrator(&viewModel);
    QSignalSpy statusSpy(&orchestrator, &ThreadOrchestrator::connectionStatusChanged);

    orchestrator.startAll();

    orchestrator.onConnectRequested();
    QVERIFY(QTest::qWaitFor([&statusSpy]() { return statusSpy.count() >= 1; },
                            TestConstants::ORCHESTRATOR_SIGNAL_WAIT_MS));
    QCOMPARE(connectionStatusFromSpy(statusSpy, 0), Telemetry::ConnectionStatus::Started);

    orchestrator.onStopGenerationRequested();
    QVERIFY(QTest::qWaitFor([&statusSpy]() { return statusSpy.count() >= 2; },
                            TestConstants::ORCHESTRATOR_SIGNAL_WAIT_MS));
    QCOMPARE(connectionStatusFromSpy(statusSpy, 1), Telemetry::ConnectionStatus::Stopped);

    orchestrator.stopAll();
}
