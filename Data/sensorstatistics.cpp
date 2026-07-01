#include "Data/sensorstatistics.h"

#include <cmath>

int SensorStatistics::connectedCount() const
{
    return m_connectedCount;
}

double SensorStatistics::averageValue() const
{
    return m_averageValue;
}

double SensorStatistics::minimumValue() const
{
    return m_minimumValue;
}

double SensorStatistics::maximumValue() const
{
    return m_maximumValue;
}

void SensorStatistics::setConnectedCount(int count)
{
    if (m_connectedCount == count) {
        return;
    }
    m_connectedCount = count;
}

void SensorStatistics::setAverageValue(double value)
{
    if (std::fabs(m_averageValue - value) < 1e-9) {
        return;
    }
    m_averageValue = value;
}

void SensorStatistics::setMinimumValue(double value)
{
    if (std::fabs(m_minimumValue - value) < 1e-9) {
        return;
    }
    m_minimumValue = value;
}

void SensorStatistics::setMaximumValue(double value)
{
    if (std::fabs(m_maximumValue - value) < 1e-9) {
        return;
    }
    m_maximumValue = value;
}
