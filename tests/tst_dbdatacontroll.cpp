#include "tst_dbdatacontroll.h"
#include "testhelpers.h"
#include "Domain/filterqueryspec.h"
#include "Infrastructure/Persistence/dbdatacontroll.h"

#include <QDateTime>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

void TestDBDataControll::init()
{
    tempDir.reset(new QTemporaryDir());
    QVERIFY(tempDir->isValid());

    const QString dbPath = tempDir->filePath(QStringLiteral("test_telemetry.db"));
    controller.reset(new DBDataControll(dbPath));
    controller->initializeDatabase();
}

void TestDBDataControll::cleanup()
{
    controller.reset();
    tempDir.reset();
}

void TestDBDataControll::initializeDatabase_createsPersistentSchema()
{
    QVERIFY(controller);

    QSignalSpy statsSpy(controller.data(), &DBDataControll::sensorStatisticsLoaded);
    controller->fetchSensorStatistics();
    QCOMPARE(statsSpy.count(), 1);
}

void TestDBDataControll::saveAndFetch_sortedByTimestamp()
{
    const QVector<SensorData> batch = {
        TestHelpers::makeRecord(0, 1, 10.0, 1000),
        TestHelpers::makeRecord(0, 2, 20.0, 2000),
        TestHelpers::makeRecord(0, 1, 30.0, 3000),
    };

    controller->onSaveBatchToSql(batch);

    QSignalSpy dataSpy(controller.data(), &DBDataControll::dataLoaded);
    controller->fetchSortedWindow(static_cast<int>(Telemetry::Column::Timestamp),
                                  static_cast<int>(Qt::AscendingOrder),
                                  10);
    QCOMPARE(dataSpy.count(), 1);

    const auto loaded = dataSpy.at(0).at(0).value<QVector<SensorData>>();
    QCOMPARE(loaded.size(), 3);
    QCOMPARE(loaded.first().timestamp, 1000u);
    QCOMPARE(loaded.last().timestamp, 3000u);
}

void TestDBDataControll::clearDatabase_removesRows()
{
    controller->onSaveBatchToSql({TestHelpers::makeRecord(0, 1, 5.0, 5000)});

    QSignalSpy clearedSpy(controller.data(), &DBDataControll::databaseCleared);
    controller->clearDatabase();
    QCOMPARE(clearedSpy.count(), 1);

    QSignalSpy dataSpy(controller.data(), &DBDataControll::dataLoaded);
    controller->fetchSortedWindow(static_cast<int>(Telemetry::Column::Timestamp),
                                  static_cast<int>(Qt::AscendingOrder),
                                  10);
    QCOMPARE(dataSpy.count(), 1);
    QCOMPARE(dataSpy.at(0).at(0).value<QVector<SensorData>>().size(), 0);
}

void TestDBDataControll::fetchSensorStatistics_aggregatesSavedBatch()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    controller->onSaveBatchToSql({
        TestHelpers::makeRecord(0, 1, 10.0, nowMs),
        TestHelpers::makeRecord(0, 2, 30.0, nowMs),
        TestHelpers::makeRecord(0, 3, 20.0, nowMs),
    });

    QSignalSpy statsSpy(controller.data(), &DBDataControll::sensorStatisticsLoaded);
    controller->fetchSensorStatistics();
    QCOMPARE(statsSpy.count(), 1);

    const SensorStatistics stats = statsSpy.at(0).at(0).value<SensorStatistics>();
    QCOMPARE(stats.connectedCount(), 3);
    QVERIFY(TestHelpers::nearlyEqual(stats.minimumValue(), 10.0));
    QVERIFY(TestHelpers::nearlyEqual(stats.maximumValue(), 30.0));
    QVERIFY(TestHelpers::nearlyEqual(stats.averageValue(), 20.0));
}

void TestDBDataControll::applyFilterQuery_timestampRange_returnsMatchingRows()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    controller->onSaveBatchToSql({
        TestHelpers::makeRecord(0, 1, 10.0, nowMs - 1000),
        TestHelpers::makeRecord(0, 2, 20.0, nowMs - 2000),
        TestHelpers::makeRecord(0, 3, 30.0, nowMs - 3600 * 1000),
    });

    FilterQuerySpec spec;
    spec.setField(FilterQuerySpec::Field::Timestamp);
    spec.setTimestampRange(nowMs - 5000, nowMs);

    QSignalSpy dataSpy(controller.data(), &DBDataControll::dataLoaded);
    controller->applyFilterQuery(spec,
                                 static_cast<int>(Telemetry::Column::Timestamp),
                                 static_cast<int>(Qt::AscendingOrder),
                                 10);
    QCOMPARE(dataSpy.count(), 1);
    QCOMPARE(dataSpy.at(0).at(0).value<QVector<SensorData>>().size(), 2);
}
