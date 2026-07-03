#include "devicereceiver.h"

DeviceReceiver::DeviceReceiver(QObject *parent)
    : QObject(parent)
    , m_flushTimer(this)
{
    connect(&m_flushTimer, &QTimer::timeout, this, &DeviceReceiver::flushData);
}

void DeviceReceiver::startProcessing()
{
    if (!m_flushTimer.isActive()) {
        m_flushTimer.start(FLUSH_INTERVAL_MS);
    }
}

void DeviceReceiver::stopProcessing()
{
    m_flushTimer.stop();
    flushData();
}

void DeviceReceiver::onRawDataReceived(SensorDataBatch rawBatch)
{
    if (!rawBatch || rawBatch->isEmpty()) {
        return;
    }

    m_localBuffer.append(*rawBatch);

    // Предохранитель по объему памяти
    if (m_localBuffer.size() >= BUFFER_FLUSH_THRESHOLD) {
        flushData();
    }
}

void DeviceReceiver::flushData() {
    if (m_localBuffer.isEmpty()) return;

    emit dataBatchReady(makeSensorDataBatch(std::move(m_localBuffer)));
    m_localBuffer.clear();
}

