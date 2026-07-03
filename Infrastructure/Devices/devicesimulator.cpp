#include "devicesimulator.h"

DeviceSimulator::DeviceSimulator(QObject *parent)
    : QObject(parent)
    , m_timer(this)
{
    connect(&m_timer, &QTimer::timeout, this, &DeviceSimulator::generateDataTick);
}

void DeviceSimulator::startGeneration()
{
    if (m_timer.isActive()) {
        return;
    }

    m_timer.start(GENERATION_INTERVAL_MS);
    emit connectionStatusChanged(Telemetry::ConnectionStatus::Started);
}

void DeviceSimulator::stopGeneration()
{
    m_timer.stop();
    emit connectionStatusChanged(Telemetry::ConnectionStatus::Stopped);
}

void DeviceSimulator::generateDataTick()
{
    QVector<SensorData> batch;
    batch.reserve(GENERATION_BATCH_SIZE);

    const uint64_t currentTimestamp = QDateTime::currentMSecsSinceEpoch();

    for (int i = 0; i < GENERATION_BATCH_SIZE; ++i) {
        SensorData data;
        data.recordId = SensorData::DEFAULT_RECORD_ID;
        data.sensorId = QRandomGenerator::global()->bounded(MIN_SENSOR_ID, EXCLUSIVE_MAX_SENSOR_ID);
        data.timestamp = currentTimestamp;
        data.value = MIN_VOLTAGE
            + QRandomGenerator::global()->generateDouble() * (MAX_VOLTAGE - MIN_VOLTAGE);
        batch.append(data);
    }

    emit rawDataGenerated(makeSensorDataBatch(std::move(batch)));
}
