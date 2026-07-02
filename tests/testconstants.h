#ifndef TESTCONSTANTS_H
#define TESTCONSTANTS_H

#include "Domain/filterlimits.h"
#include "Domain/sensordata.h"
#include "Domain/sensorlimits.h"

#include <cstdint>

namespace TestConstants {

inline constexpr double DOUBLE_EPSILON = 1e-9;

inline constexpr uint64_t DEFAULT_SENSOR_ID = 1;
inline constexpr double DEFAULT_SENSOR_VALUE = 10.0;
inline constexpr uint64_t DEFAULT_TIMESTAMP_MS = 1000;
inline constexpr int DEFAULT_QUERY_LIMIT = 10;

inline constexpr double OVER_MAX_SENSOR_VALUE = SensorLimits::MAX_VOLTAGE_VOLTS + 20.0;
inline constexpr double BELOW_MIN_SENSOR_VALUE = -5.0;
inline constexpr double TRUNCATE_VALUE_INPUT = 12.349;
inline constexpr double TRUNCATE_VALUE_EXPECTED = 12.34;

inline constexpr double OVER_MAX_TOLERANCE = 2.0;
inline constexpr double BELOW_MIN_TOLERANCE = -1.0;
inline constexpr double TRUNCATE_TOLERANCE_INPUT = 0.123456;
inline constexpr double TRUNCATE_TOLERANCE_EXPECTED = 0.1234;

inline constexpr double SMALL_TOLERANCE_INPUT = 0.001;
inline constexpr double LARGE_TOLERANCE_INPUT = 0.05;

inline constexpr int SAMPLE_SENSOR_ID = 42;
inline constexpr int OTHER_SENSOR_ID = 5;
inline constexpr int FILTER_SPEC_SENSOR_ID = 7;

inline constexpr double NEAR_FILTER_VALUE = 100.0;
inline constexpr double NEAR_FILTER_TOLERANCE = 0.5;
inline constexpr double BOUNDARY_FILTER_VALUE = 50.0;
inline constexpr double BOUNDARY_FILTER_TOLERANCE = 0.1;
inline constexpr double FILTER_SPEC_TOLERANCE = 0.25;

inline constexpr qint64 TIMESTAMP_RANGE_FROM_MS = 1000;
inline constexpr qint64 TIMESTAMP_RANGE_TO_MS = 5000;

inline constexpr int TEST_YEAR = 2026;
inline constexpr int TEST_MONTH = 7;
inline constexpr int TEST_DAY = 2;
inline constexpr int TEST_FROM_HOUR = 14;
inline constexpr int TEST_FROM_MINUTE = 1;
inline constexpr int TEST_TO_HOUR = 16;
inline constexpr int TEST_TO_MINUTE = 1;

inline constexpr double STATS_AVERAGE = 12.34;
inline constexpr double STATS_MINIMUM = 1.0;
inline constexpr double STATS_MAXIMUM = 99.9;
inline constexpr int STATS_CONNECTED_COUNT = 3;

inline constexpr double DB_VALUE_LOW = 10.0;
inline constexpr double DB_VALUE_MID = 20.0;
inline constexpr double DB_VALUE_HIGH = 30.0;
inline constexpr double DB_VALUE_EXTRA = 5.0;
inline constexpr int DB_SENSOR_ID_FIRST = 1;
inline constexpr int DB_SENSOR_ID_SECOND = 2;
inline constexpr int DB_SENSOR_ID_THIRD = 3;
inline constexpr uint64_t DB_TIMESTAMP_FIRST = 1000;
inline constexpr uint64_t DB_TIMESTAMP_SECOND = 2000;
inline constexpr uint64_t DB_TIMESTAMP_THIRD = 3000;
inline constexpr uint64_t DB_TIMESTAMP_EXTRA = 5000;
inline constexpr int DB_EXPECTED_ROW_COUNT = 3;
inline constexpr int DB_OFFSET_ONE_SECOND_MS = 1000;
inline constexpr int DB_OFFSET_TWO_SECONDS_MS = 2000;

inline constexpr double STATS_VM_AVERAGE = 10.5;
inline constexpr double STATS_VM_MINIMUM = 1.25;
inline constexpr double STATS_VM_MAXIMUM = 20.0;
inline constexpr int STATS_VM_CONNECTED = 2;
inline constexpr double STATS_VM_UNIFORM = 5.0;
inline constexpr int STATS_VM_UNIFORM_CONNECTED = 1;

inline constexpr quint64 TELEMETRY_RECORD_FIRST = 1;
inline constexpr quint64 TELEMETRY_RECORD_SECOND = 2;
inline constexpr quint64 TELEMETRY_RECORD_THIRD = 3;
inline constexpr quint64 TELEMETRY_RECORD_TEN = 10;
inline constexpr quint64 TELEMETRY_RECORD_ELEVEN = 11;

} // namespace TestConstants

#endif // TESTCONSTANTS_H
