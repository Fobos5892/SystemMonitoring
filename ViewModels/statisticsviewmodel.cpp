#include "statisticsviewmodel.h"
#include "Domain/telemetrytypes.h"

StatisticsViewModel::StatisticsViewModel(QObject *parent)
    : QObject(parent)
{
}

void StatisticsViewModel::updateStatistics(const SensorStatistics &stats)
{
    const QString connected =
        tr("Активных датчиков(за %1 минут): %2")
            .arg(Telemetry::SENSOR_ACTIVITY_WINDOW_MINUTES)
            .arg(stats.connectedCount());
    const QString average = tr("Среднее: %1 В")
                                .arg(stats.averageValue(), 0, 'f',
                                     Telemetry::VOLTAGE_DISPLAY_DECIMAL_PLACES);
    const QString minimum = tr("Минимум: %1 В")
                                .arg(stats.minimumValue(), 0, 'f',
                                     Telemetry::VOLTAGE_DISPLAY_DECIMAL_PLACES);
    const QString maximum = tr("Максимум: %1 В")
                                .arg(stats.maximumValue(), 0, 'f',
                                     Telemetry::VOLTAGE_DISPLAY_DECIMAL_PLACES);

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
