#ifndef SENSORSTATISTICS_H
#define SENSORSTATISTICS_H

class SensorStatistics {
public:
    static constexpr int DEFAULT_CONNECTED_COUNT = 0;
    static constexpr double DEFAULT_AVERAGE_VALUE = 0.0;
    static constexpr double DEFAULT_MINIMUM_VALUE = 0.0;
    static constexpr double DEFAULT_MAXIMUM_VALUE = 0.0;

    SensorStatistics() = default;

    int connectedCount() const;
    double averageValue() const;
    double minimumValue() const;
    double maximumValue() const;

    void setConnectedCount(int count);
    void setAverageValue(double value);
    void setMinimumValue(double value);
    void setMaximumValue(double value);

private:
    int m_connectedCount = DEFAULT_CONNECTED_COUNT;
    double m_averageValue = DEFAULT_AVERAGE_VALUE;
    double m_minimumValue = DEFAULT_MINIMUM_VALUE;
    double m_maximumValue = DEFAULT_MAXIMUM_VALUE;
};

#endif // SENSORSTATISTICS_H
