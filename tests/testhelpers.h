#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#include "Domain/sensordata.h"

#include <cmath>

namespace TestHelpers {

inline constexpr double kDoubleEpsilon = 1e-9;

inline SensorData makeRecord(quint64 recordId, uint64_t sensorId = 1, double value = 10.0,
                             uint64_t timestamp = 1000)
{
    return {recordId, sensorId, timestamp, value};
}

inline bool nearlyEqual(double lhs, double rhs, double epsilon = kDoubleEpsilon)
{
    return std::abs(lhs - rhs) < epsilon;
}

} // namespace TestHelpers

#endif // TESTHELPERS_H
