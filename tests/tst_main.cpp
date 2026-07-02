#include "tst_dbdatacontroll.h"
#include "tst_devicesimulator.h"
#include "tst_filterqueryspec.h"
#include "tst_filterviewmodel.h"
#include "tst_sensorstatistics.h"
#include "tst_statisticsviewmodel.h"
#include "tst_telemetryviewmodel.h"
#include "tst_telemetrymergelogic.h"
#include "tst_threadorchestrator.h"

#include "Domain/metatypes.h"

#include <QCoreApplication>
#include <QDebug>
#include <QtTest>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<SensorData>();
    qRegisterMetaType<SensorStatistics>();
    qRegisterMetaType<QVector<SensorData>>();
    qRegisterMetaType<FilterQuerySpec>();
    qRegisterMetaType<Telemetry::AnchorSide>();
    qRegisterMetaType<Telemetry::ConnectionStatus>();
    qRegisterMetaType<Telemetry::ViewportZone>();

    int status = 0;
    int passedSuites = 0;
    int failedSuites = 0;
    int totalFailedChecks = 0;

    {
        TestSensorStatistics suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestFilterQuerySpec suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestFilterViewModel suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestStatisticsViewModel suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestTelemetryViewModel suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestDeviceSimulator suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestThreadOrchestrator suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestTelemetryMergeLogic suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }
    {
        TestDBDataControll suite;
        const int suiteStatus = QTest::qExec(&suite, argc, argv);
        status |= suiteStatus;
        totalFailedChecks += suiteStatus;
        suiteStatus == 0 ? ++passedSuites : ++failedSuites;
    }

    qInfo().noquote() << QStringLiteral(
                             "\n======== Overall Test Summary ========\n"
                             "Suites passed: %1\n"
                             "Suites failed: %2\n"
                             "Failed checks: %3\n"
                             "======================================")
                             .arg(passedSuites)
                             .arg(failedSuites)
                             .arg(totalFailedChecks);

    return status;
}
