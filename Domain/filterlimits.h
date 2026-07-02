#ifndef FILTERLIMITS_H
#define FILTERLIMITS_H

#include "sensorlimits.h"

namespace FilterLimits {

static constexpr double MIN_SENSOR_VALUE_VOLTS = SensorLimits::MIN_VOLTAGE_VOLTS;
static constexpr double MAX_SENSOR_VALUE_VOLTS = SensorLimits::MAX_VOLTAGE_VOLTS;
static constexpr int SENSOR_VALUE_DECIMAL_PLACES = 2;

static constexpr double MIN_TOLERANCE = 0.0;
static constexpr double MAX_TOLERANCE = 1.0;
static constexpr int TOLERANCE_DECIMAL_PLACES = 4;

static constexpr double SMALL_TOLERANCE_THRESHOLD = 0.01;
static constexpr double FINE_TOLERANCE_STEP = 0.0001;
static constexpr double COARSE_TOLERANCE_STEP = 0.01;
static constexpr double TOLERANCE_SPINBOX_SINGLE_STEP = COARSE_TOLERANCE_STEP;

static constexpr int MAX_SENSOR_ID = 999999;

} // namespace FilterLimits

#endif // FILTERLIMITS_H
