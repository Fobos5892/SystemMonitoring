#include "tst_dbdatacontroll.h"
#include "tst_filterviewmodel.h"
#include "tst_sensorstatistics.h"
#include "tst_statisticsviewmodel.h"
#include "tst_telemetryviewmodel.h"

#include "Domain/metatypes.h"

#include <QCoreApplication>
#include <QtTest>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qRegisterMetaType<SensorData>();
    qRegisterMetaType<SensorStatistics>();
    qRegisterMetaType<QVector<SensorData>>();
    qRegisterMetaType<Telemetry::AnchorSide>();

    int status = 0;

    {
        TestSensorStatistics suite;
        status |= QTest::qExec(&suite, argc, argv);
    }
    {
        TestFilterViewModel suite;
        status |= QTest::qExec(&suite, argc, argv);
    }
    {
        TestStatisticsViewModel suite;
        status |= QTest::qExec(&suite, argc, argv);
    }
    {
        TestTelemetryViewModel suite;
        status |= QTest::qExec(&suite, argc, argv);
    }
    {
        TestDBDataControll suite;
        status |= QTest::qExec(&suite, argc, argv);
    }

    return status;
}
