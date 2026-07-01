#ifndef DEVICERECEIVER_H
#define DEVICERECEIVER_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include "Data/sensordata.h"

class DeviceReceiver : public QObject {
    Q_OBJECT
public:
    explicit DeviceReceiver(QObject *parent = nullptr);

signals:
    // Сигнал для отправки очищенного и упакованного пакета дальше по конвейеру
    void dataBatchReady(const QVector<SensorData> &batch);

public slots:
    // Слот принимает сырые пачки от нашего нового класса-генератора
    void onRawDataReceived(const QVector<SensorData> &rawBatch);
    void startProcessing();
    void stopProcessing();

private:
    void flushData();

    QVector<SensorData> m_localBuffer;
    QTimer m_flushTimer;
};

#endif // DEVICERECEIVER_H
