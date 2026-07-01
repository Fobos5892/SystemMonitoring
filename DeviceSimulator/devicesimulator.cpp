#include "devicesimulator.h"

DeviceSimulator::DeviceSimulator(QObject *parent)
    : QObject(parent)
    , m_timer(this)
{
    connect(&m_timer, &QTimer::timeout, this, &DeviceSimulator::generateDataTick);
}

void DeviceSimulator::startGeneration()
{
    m_timer.start(30);
}

void DeviceSimulator::stopGeneration()
{
    m_timer.stop();
}

void DeviceSimulator::generateDataTick()
{
    QVector<SensorData> batch;
    batch.reserve(50);

    const uint64_t currentTimestamp = QDateTime::currentMSecsSinceEpoch();

    for (int i = 0; i < 50; ++i) {
        SensorData data;
        data.recordId = 0;
        data.sensorId = QRandomGenerator::global()->bounded(1, 10001);
        data.timestamp = currentTimestamp;
        const double noise = QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
        data.value = 50.0 + noise;
        batch.append(data);
    }

    emit rawDataGenerated(batch);
}
