#include "Domain/sensorstatistics.h"

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
    m_connectedCount = count;
}

void SensorStatistics::setAverageValue(double value)
{
    m_averageValue = value;
}

void SensorStatistics::setMinimumValue(double value)
{
    m_minimumValue = value;
}

void SensorStatistics::setMaximumValue(double value)
{
    m_maximumValue = value;
}
