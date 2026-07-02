#include "filterqueryspec.h"

#include <QtGlobal>

FilterQuerySpec::Field FilterQuerySpec::field() const
{
    return m_field;
}

int FilterQuerySpec::sensorId() const
{
    return m_sensorId;
}

double FilterQuerySpec::value() const
{
    return m_value;
}

double FilterQuerySpec::tolerance() const
{
    return m_tolerance;
}

FilterQuerySpec::ValueOperation FilterQuerySpec::valueOperation() const
{
    return m_valueOperation;
}

qint64 FilterQuerySpec::fromTimestampMs() const
{
    return m_fromTimestampMs;
}

qint64 FilterQuerySpec::toTimestampMs() const
{
    return m_toTimestampMs;
}

void FilterQuerySpec::setField(Field field)
{
    m_field = field;
}

void FilterQuerySpec::setSensorId(int sensorId)
{
    m_sensorId = sensorId;
}

void FilterQuerySpec::setValue(double value)
{
    m_value = value;
}

void FilterQuerySpec::setTolerance(double tolerance)
{
    m_tolerance = tolerance;
}

void FilterQuerySpec::setValueOperation(ValueOperation operation)
{
    m_valueOperation = operation;
}

void FilterQuerySpec::setTimestampRange(qint64 fromMs, qint64 toMs)
{
    m_fromTimestampMs = qMin(fromMs, toMs);
    m_toTimestampMs = qMax(fromMs, toMs);
}

QString FilterQuerySpec::toSqlCondition(const QString &tableAlias) const
{
    const auto column = [&tableAlias](const char *name) {
        return tableAlias.isEmpty() ? QString::fromLatin1(name)
                                    : tableAlias + QLatin1Char('.') + QLatin1String(name);
    };

    switch (m_field) {
    case Field::SensorId:
        return QStringLiteral("%1 = %2").arg(column("sensor_id")).arg(m_sensorId);
    case Field::Value: {
        const QString valueSql = QString::number(m_value, 'f', SENSOR_VALUE_DECIMAL_PLACES);
        const QString toleranceSql = QString::number(m_tolerance, 'f', TOLERANCE_DECIMAL_PLACES);
        const QString valueColumn = column("value");

        switch (m_valueOperation) {
        case ValueOperation::Near:
            return QStringLiteral("ABS(%1 - %2) <= %3").arg(valueColumn, valueSql, toleranceSql);
        case ValueOperation::Greater:
            return QStringLiteral("%1 > %2")
                .arg(valueColumn,
                     QString::number(m_value + m_tolerance, 'f', TOLERANCE_DECIMAL_PLACES));
        case ValueOperation::Less:
            return QStringLiteral("%1 < %2")
                .arg(valueColumn,
                     QString::number(m_value - m_tolerance, 'f', TOLERANCE_DECIMAL_PLACES));
        }
        return {};
    }
    case Field::Timestamp:
        return QStringLiteral("%1 BETWEEN %2 AND %3")
            .arg(column("timestamp"))
            .arg(m_fromTimestampMs)
            .arg(m_toTimestampMs);
    }
    return {};
}
