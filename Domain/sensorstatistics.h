#ifndef SENSORSTATISTICS_H
#define SENSORSTATISTICS_H

class SensorStatistics {
public:
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
    int m_connectedCount = 0;
    double m_averageValue = 0.0;
    double m_minimumValue = 0.0;
    double m_maximumValue = 0.0;
};

#endif // SENSORSTATISTICS_H
