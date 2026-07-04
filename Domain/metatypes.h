#ifndef METATYPES_H
#define METATYPES_H

#include <QMetaType>
#include <QSharedPointer>
#include <QVector>

#include "Domain/sensordata.h"
#include "Domain/sensordatabatch.h"
#include "Domain/sensorstatistics.h"
#include "Domain/filterqueryspec.h"
#include "Domain/telemetrytypes.h"

Q_DECLARE_METATYPE(SensorData)
Q_DECLARE_METATYPE(SensorStatistics)
Q_DECLARE_METATYPE(QVector<SensorData>)
// Qt 5: type alias is not visible to QMetaType; declare the concrete type.
Q_DECLARE_METATYPE(QSharedPointer<const QVector<SensorData>>)
Q_DECLARE_METATYPE(Telemetry::AnchorSide)
Q_DECLARE_METATYPE(Telemetry::ConnectionStatus)
Q_DECLARE_METATYPE(Telemetry::ViewportZone)

inline void registerDomainMetaTypes()
{
    qRegisterMetaType<SensorData>("SensorData");
    qRegisterMetaType<SensorStatistics>("SensorStatistics");
    qRegisterMetaType<QVector<SensorData>>("QVector<SensorData>");
    // Qt 5: using-alias is invisible to qRegisterMetaType<SensorDataBatch>().
    qRegisterMetaType<QSharedPointer<const QVector<SensorData>>>("SensorDataBatch");
    qRegisterMetaType<FilterQuerySpec>("FilterQuerySpec");
    qRegisterMetaType<Telemetry::AnchorSide>("Telemetry::AnchorSide");
    qRegisterMetaType<Telemetry::ConnectionStatus>("Telemetry::ConnectionStatus");
    qRegisterMetaType<Telemetry::ViewportZone>("Telemetry::ViewportZone");
}

#endif // METATYPES_H
