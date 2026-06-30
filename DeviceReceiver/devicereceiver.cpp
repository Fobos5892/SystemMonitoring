#include "devicereceiver.h"

DeviceReceiver::DeviceReceiver(QObject *parent) : QObject(parent)
{
    connect(&m_flushTimer, &QTimer::timeout, this, &DeviceReceiver::flushData);
    m_flushTimer.start(40); // Сброс пачки на UI и в БД 25 раз в секунду
}

void DeviceReceiver::onRawDataReceived(const QVector<SensorData> &rawBatch)
{
    m_localBuffer.append(rawBatch);

    // Предохранитель по объему памяти
    if (m_localBuffer.size() >= 1000) {
        flushData();
    }
}

void DeviceReceiver::flushData() {
    if (m_localBuffer.isEmpty()) return;

    emit dataBatchReady(m_localBuffer);
    m_localBuffer.clear();
}

