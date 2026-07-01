#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <cstdint>

struct SensorData {
    uint64_t recordId = 0;   // PK в SQLite (id записи в telemetry)
    uint64_t sensorId = 0;   // ID устройства/датчика
    uint64_t timestamp = 0;
    double   value = 0.0;
};

#endif // SENSORDATA_H
