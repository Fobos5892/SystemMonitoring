#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <cstdint>

struct SensorData {
    uint64_t id;
    uint64_t timestamp;
    double  value;
};

#endif // SENSORDATA_H
