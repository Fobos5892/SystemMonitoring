#include "devicesimulator.h"

DeviceSimulator::DeviceSimulator(QObject *parent) : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &DeviceSimulator::generateDataTick);
}

void DeviceSimulator::startGeneration()
{
    m_timer.start(10); // Высокая интенсивность: генерация каждые 10 мс
}

void DeviceSimulator::stopGeneration()
{
    m_timer.stop();
}

void DeviceSimulator::generateDataTick()
{
    QVector<SensorData> batch;
    // Имитируем, что за 10 мс прилетело, например, 50 сигналов от разных устройств
    batch.reserve(50);

    uint64_t currentTimestamp = QDateTime::currentMSecsSinceEpoch();

    for (int i = 0; i < 50; ++i) {
        SensorData data;
        // id = 0, так как окончательный инкремент-ID присвоит SQLite при сохранении
        data.id = 0;
        // Случайный ID устройства из 10 000 доступных
        data.id = QRandomGenerator::global()->bounded(1, 10001);
        data.timestamp = currentTimestamp;
        // Симулируем синусоиду с шумом для красивого графика/значений
        double noise = QRandomGenerator::global()->generateDouble() * 2.0 - 1.0;
        data.value = 50.0 + noise;

        batch.append(data);
    }

    emit rawDataGenerated(batch);
}