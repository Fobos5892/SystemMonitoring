#ifndef METATYPES_H
#define METATYPES_H

#include <QMetaType>
#include <QVector>

#include "Data/sensordata.h"
#include "Data/sensorstatistics.h"
#include "DBModel/dbdatacontroll.h"

Q_DECLARE_METATYPE(SensorData)
Q_DECLARE_METATYPE(SensorStatistics)
Q_DECLARE_METATYPE(QVector<SensorData>)
Q_DECLARE_METATYPE(DBDataControll::AnchorSide)

#endif // METATYPES_H
