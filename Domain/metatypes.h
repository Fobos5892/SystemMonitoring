#ifndef METATYPES_H
#define METATYPES_H

#include <QMetaType>
#include <QVector>

#include "Domain/sensordata.h"
#include "Domain/sensorstatistics.h"
#include "Domain/filterqueryspec.h"
#include "Domain/telemetrytypes.h"

Q_DECLARE_METATYPE(SensorData)
Q_DECLARE_METATYPE(SensorStatistics)
Q_DECLARE_METATYPE(QVector<SensorData>)
Q_DECLARE_METATYPE(Telemetry::AnchorSide)

#endif // METATYPES_H
