#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H

#include "Domain/sensorlimits.h"
#include "Domain/telemetrytypes.h"
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QRandomGenerator>
#include <QDateTime>
#include "Domain/sensordata.h"

class DeviceSimulator : public QObject {
    Q_OBJECT
public:
    static constexpr double MIN_VOLTAGE = SensorLimits::MIN_VOLTAGE_VOLTS;
    static constexpr double MAX_VOLTAGE = SensorLimits::MAX_VOLTAGE_VOLTS;

    static constexpr int GENERATION_BATCH_SIZE = 50;
    static constexpr int GENERATION_INTERVAL_MS = 30;
    static constexpr int MIN_SENSOR_ID = 1;
    static constexpr int EXCLUSIVE_MAX_SENSOR_ID = 10001;

    explicit DeviceSimulator(QObject *parent = nullptr);

signals:
    // Сигнал передает сгенерированную пачку данных в ресивер
    void rawDataGenerated(const QVector<SensorData> &rawBatch);
    void connectionStatusChanged(Telemetry::ConnectionStatus status);

public slots:
    void startGeneration();
    void stopGeneration();

private:
    void generateDataTick();

    QTimer m_timer;
};

#endif // DEVICESIMULATOR_H
