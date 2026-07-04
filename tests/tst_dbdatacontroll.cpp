#include "tst_dbdatacontroll.h"
#include "Domain/metatypes.h"
#include "Domain/sensorstatistics.h"
#include "Domain/telemetrytypes.h"
#include "testconstants.h"
#include "testhelpers.h"
#include "Domain/filterqueryspec.h"
#include "Domain/sensordatabatch.h"
#include "Infrastructure/Persistence/dbdatacontroll.h"
#include "ViewModels/filterviewmodel.h"

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
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_FIRST,
                                TestConstants::DB_VALUE_LOW, TestConstants::DB_TIMESTAMP_FIRST),
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_SECOND,
                                TestConstants::DB_VALUE_MID, TestConstants::DB_TIMESTAMP_SECOND),
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_FIRST,
                                TestConstants::DB_VALUE_HIGH, TestConstants::DB_TIMESTAMP_THIRD),
    };

    controller->onSaveBatchToSql(makeSensorDataBatch(batch));

    QSignalSpy dataSpy(controller.data(), &DBDataControll::dataLoaded);
    controller->fetchSortedWindow(static_cast<int>(Telemetry::Column::Timestamp),
                                  static_cast<int>(Qt::AscendingOrder),
                                  TestConstants::DEFAULT_QUERY_LIMIT);
    QCOMPARE(dataSpy.count(), 1);

    const auto loaded = dataSpy.at(0).at(0).value<SensorDataBatch>();
    QVERIFY(loaded);
    QCOMPARE(loaded->size(), TestConstants::DB_EXPECTED_ROW_COUNT);
    QCOMPARE(loaded->first().timestamp, TestConstants::DB_TIMESTAMP_FIRST);
    QCOMPARE(loaded->last().timestamp, TestConstants::DB_TIMESTAMP_THIRD);
}

void TestDBDataControll::clearDatabase_removesRows()
{
    controller->onSaveBatchToSql(makeSensorDataBatch({TestHelpers::makeRecord(
        SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_FIRST,
        TestConstants::DB_VALUE_EXTRA, TestConstants::DB_TIMESTAMP_EXTRA)}));

    QSignalSpy clearedSpy(controller.data(), &DBDataControll::databaseCleared);
    controller->clearDatabase();
    QCOMPARE(clearedSpy.count(), 1);

    QSignalSpy dataSpy(controller.data(), &DBDataControll::dataLoaded);
    controller->fetchSortedWindow(static_cast<int>(Telemetry::Column::Timestamp),
                                  static_cast<int>(Qt::AscendingOrder),
                                  TestConstants::DEFAULT_QUERY_LIMIT);
    QCOMPARE(dataSpy.count(), 1);
    QCOMPARE(dataSpy.at(0).at(0).value<SensorDataBatch>()->size(), 0);
}

void TestDBDataControll::fetchSensorStatistics_aggregatesSavedBatch()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    controller->onSaveBatchToSql(makeSensorDataBatch({
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_FIRST,
                                TestConstants::DB_VALUE_LOW, nowMs),
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_SECOND,
                                TestConstants::DB_VALUE_HIGH, nowMs),
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_THIRD,
                                TestConstants::DB_VALUE_MID, nowMs),
    }));

    QSignalSpy statsSpy(controller.data(), &DBDataControll::sensorStatisticsLoaded);
    controller->fetchSensorStatistics();
    QCOMPARE(statsSpy.count(), 1);

    const SensorStatistics stats = statsSpy.at(0).at(0).value<SensorStatistics>();
    QCOMPARE(stats.connectedCount(), TestConstants::DB_EXPECTED_ROW_COUNT);
    QVERIFY(TestHelpers::nearlyEqual(stats.minimumValue(), TestConstants::DB_VALUE_LOW));
    QVERIFY(TestHelpers::nearlyEqual(stats.maximumValue(), TestConstants::DB_VALUE_HIGH));
    QVERIFY(TestHelpers::nearlyEqual(stats.averageValue(), TestConstants::DB_VALUE_MID));
}

void TestDBDataControll::applyFilterQuery_timestampRange_returnsMatchingRows()
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    controller->onSaveBatchToSql(makeSensorDataBatch({
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_FIRST,
                                TestConstants::DB_VALUE_LOW,
                                nowMs - TestConstants::DB_OFFSET_ONE_SECOND_MS),
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_SECOND,
                                TestConstants::DB_VALUE_MID,
                                nowMs - TestConstants::DB_OFFSET_TWO_SECONDS_MS),
        TestHelpers::makeRecord(SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_THIRD,
                                TestConstants::DB_VALUE_HIGH, nowMs - Telemetry::HOUR_MS),
    }));

    FilterViewModel vm;
    vm.setField(FilterViewModel::Field::Timestamp);
    vm.setTimestampRange(
        FilterViewModel::combineLocalDateTime(QDate::currentDate(), FilterViewModel::DAY_START_TIME),
        FilterViewModel::combineLocalDateTime(QDate::currentDate(), QTime::currentTime(), true));
    const FilterQuerySpec spec = vm.buildQuerySpec();

    QSignalSpy dataSpy(controller.data(), &DBDataControll::dataLoaded);
    controller->applyFilterQuery(spec,
                                 static_cast<int>(Telemetry::Column::Timestamp),
                                 static_cast<int>(Qt::AscendingOrder),
                                 TestConstants::DEFAULT_QUERY_LIMIT);
    QCOMPARE(dataSpy.count(), 1);
    QCOMPARE(dataSpy.at(0).at(0).value<SensorDataBatch>()->size(),
             TestConstants::DB_EXPECTED_ROW_COUNT);
}

void TestDBDataControll::onSaveBatchToSql_emitsBatchCommittedWithRecordIds()
{
    QSignalSpy batchSpy(controller.data(), &DBDataControll::batchCommitted);
    controller->onSaveBatchToSql(makeSensorDataBatch({TestHelpers::makeRecord(
        SensorData::DEFAULT_RECORD_ID, TestConstants::DB_SENSOR_ID_FIRST,
        TestConstants::DB_VALUE_LOW, TestConstants::DB_TIMESTAMP_FIRST)}));

    QCOMPARE(batchSpy.count(), 1);
    const SensorDataBatch inserted = batchSpy.at(0).at(0).value<SensorDataBatch>();
    QVERIFY(inserted);
    QCOMPARE(inserted->size(), 1);
    QVERIFY(inserted->first().recordId > SensorData::DEFAULT_RECORD_ID);
    QCOMPARE(inserted->first().sensorId, static_cast<uint64_t>(TestConstants::DB_SENSOR_ID_FIRST));
}
