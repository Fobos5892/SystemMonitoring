#include "statisticsviewmodel.h"

StatisticsViewModel::StatisticsViewModel(QObject *parent)
    : QObject(parent)
{
}

void StatisticsViewModel::updateStatistics(const SensorStatistics &stats)
{
    const QString connected = tr("Активных датчиков: %1").arg(stats.connectedCount());
    const QString average = tr("Среднее: %1 В").arg(stats.averageValue(), 0, 'f', 2);
    const QString minimum = tr("Минимум: %1 В").arg(stats.minimumValue(), 0, 'f', 2);
    const QString maximum = tr("Максимум: %1 В").arg(stats.maximumValue(), 0, 'f', 2);

    if (connectedLabelText == connected
        && averageLabelText == average
        && minimumLabelText == minimum
        && maximumLabelText == maximum) {
        return;
    }

    connectedLabelText = connected;
    averageLabelText = average;
    minimumLabelText = minimum;
    maximumLabelText = maximum;
    emit labelsChanged();
}
