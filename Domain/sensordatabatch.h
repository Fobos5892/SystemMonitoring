#ifndef SENSORDATABATCH_H
#define SENSORDATABATCH_H

#include "sensordata.h"

#include <QSharedPointer>
#include <QVector>

using SensorDataBatch = QSharedPointer<const QVector<SensorData>>;

inline SensorDataBatch makeSensorDataBatch(QVector<SensorData> &&data)
{
    return SensorDataBatch::create(std::move(data));
}

inline SensorDataBatch makeSensorDataBatch(const QVector<SensorData> &data)
{
    return SensorDataBatch::create(data);
}

inline SensorDataBatch emptySensorDataBatch()
{
    return SensorDataBatch::create();
}

#endif // SENSORDATABATCH_H
