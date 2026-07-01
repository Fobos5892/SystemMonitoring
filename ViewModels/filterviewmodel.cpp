#include "filterviewmodel.h"

#include <algorithm>
#include <cmath>

FilterViewModel::FilterViewModel(QObject *parent)
    : QObject(parent)
{
}

void FilterViewModel::setField(Field field)
{
    this->field = field;
}

void FilterViewModel::setSensorId(int sensorId)
{
    this->sensorId = sensorId;
}

void FilterViewModel::setValueFilter(double value, double tolerance, ValueOperation operation)
{
    this->value = value;
    this->tolerance = tolerance;
    valueOperation = operation;
}

void FilterViewModel::setTimestampRange(const QDateTime &from, const QDateTime &to)
{
    dateFrom = from;
    dateTo = to;
}

double FilterViewModel::truncateTo(double value, int decimals)
{
    const double scale = std::pow(10.0, decimals);
    return std::floor(value * scale) / scale;
}

double FilterViewModel::normalizeValue(double value)
{
    return truncateTo(std::clamp(value, 0.0, 280.0), 2);
}

double FilterViewModel::normalizeTolerance(double tolerance)
{
    return truncateTo(std::clamp(tolerance, 0.0, 1.0), 4);
}

double FilterViewModel::adaptiveToleranceStep(double tolerance)
{
    return tolerance < 0.01 ? 0.0001 : 0.01;
}

QString FilterViewModel::buildSqlCondition() const
{
    switch (field) {
    case Field::SensorId:
        return QStringLiteral("sensor_id = %1").arg(sensorId);
    case Field::Value: {
        const double normalizedValue = normalizeValue(value);
        const double normalizedTolerance = normalizeTolerance(tolerance);
        const QString valueSql = QString::number(normalizedValue, 'f', 2);
        const QString toleranceSql = QString::number(normalizedTolerance, 'f', 4);

        switch (valueOperation) {
        case ValueOperation::Near:
            return QStringLiteral("ABS(value - %1) <= %2").arg(valueSql, toleranceSql);
        case ValueOperation::Greater:
            return QStringLiteral("value > %1")
                .arg(QString::number(normalizedValue + normalizedTolerance, 'f', 4));
        case ValueOperation::Less:
            return QStringLiteral("value < %1")
                .arg(QString::number(normalizedValue - normalizedTolerance, 'f', 4));
        }
        return {};
    }
    case Field::Timestamp: {
        const qint64 fromMs = dateFrom.toMSecsSinceEpoch();
        const qint64 toMs = dateTo.toMSecsSinceEpoch();
        const qint64 startMs = qMin(fromMs, toMs);
        const qint64 endMs = qMax(fromMs, toMs);
        return QStringLiteral("timestamp BETWEEN %1 AND %2").arg(startMs).arg(endMs);
    }
    }
    return {};
}
