#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#include "Domain/sensordata.h"
#include "testconstants.h"

#include <cmath>

namespace TestHelpers {

inline constexpr double DOUBLE_EPSILON = TestConstants::DOUBLE_EPSILON;

inline SensorData makeRecord(quint64 recordId,
                             uint64_t sensorId = TestConstants::DEFAULT_SENSOR_ID,
                             double value = TestConstants::DEFAULT_SENSOR_VALUE,
                             uint64_t timestamp = TestConstants::DEFAULT_TIMESTAMP_MS)
{
    return {recordId, sensorId, timestamp, value};
}

inline bool nearlyEqual(double lhs, double rhs, double epsilon = DOUBLE_EPSILON)
{
    return std::abs(lhs - rhs) < epsilon;
}

} // namespace TestHelpers

#endif // TESTHELPERS_H
