#ifndef DEVICERECEIVER_H
#define DEVICERECEIVER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "Data/sensordata.h"

class DeviceReceiver : public QObject {
    Q_OBJECT
public:
    static constexpr int FLUSH_INTERVAL_MS = 40;
    static constexpr int BUFFER_FLUSH_THRESHOLD = 1000;

    explicit DeviceReceiver(QObject *parent = nullptr);

signals:
    // Сигнал для отправки очищенного и упакованного пакета дальше по конвейеру
    void dataBatchReady(const QVector<SensorData> &batch);

public slots:
    // Слот принимает набор сырых данных
    void onRawDataReceived(const QVector<SensorData> &rawBatch);
    void startProcessing();
    void stopProcessing();

private:
    void flushData();

    QVector<SensorData> m_localBuffer;
    QTimer m_flushTimer;
};

#endif // DEVICERECEIVER_H
