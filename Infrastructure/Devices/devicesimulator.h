#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QRandomGenerator>
#include <QDateTime>
#include "Domain/sensordata.h"

class DeviceSimulator : public QObject {
    Q_OBJECT
public:
    static constexpr double MIN_VOLTAGE = 0.0;
    static constexpr double MAX_VOLTAGE = 280.0;

    explicit DeviceSimulator(QObject *parent = nullptr);

signals:
    // Сигнал передает сгенерированную пачку данных в ресивер
    void rawDataGenerated(const QVector<SensorData> &rawBatch);

public slots:
    void startGeneration();
    void stopGeneration();

private:
    void generateDataTick();

    QTimer m_timer;
};

#endif // DEVICESIMULATOR_H
