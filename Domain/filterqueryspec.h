#ifndef FILTERQUERYSPEC_H
#define FILTERQUERYSPEC_H

#include "Domain/filterlimits.h"
#include "Domain/sensordata.h"
#include <QMetaType>
#include <QString>
#include <QtGlobal>

class FilterQuerySpec {
    Q_GADGET

public:
    enum class Field : int {
        SensorId = 0,
        Value,
        Timestamp
    };
    Q_ENUM(Field)

    enum class ValueOperation : int {
        Near = 0,
        Greater,
        Less
    };
    Q_ENUM(ValueOperation)

    FilterQuerySpec() = default;

    static constexpr int DEFAULT_SENSOR_ID = 0;
    static constexpr double DEFAULT_VALUE = 0.0;
    static constexpr double DEFAULT_TOLERANCE = 0.0;
    static constexpr qint64 DEFAULT_TIMESTAMP_MS = 0;

    static constexpr int SENSOR_VALUE_DECIMAL_PLACES = FilterLimits::SENSOR_VALUE_DECIMAL_PLACES;
    static constexpr int TOLERANCE_DECIMAL_PLACES = FilterLimits::TOLERANCE_DECIMAL_PLACES;

    Field field() const;
    int sensorId() const;
    double value() const;
    double tolerance() const;
    ValueOperation valueOperation() const;
    qint64 fromTimestampMs() const;
    qint64 toTimestampMs() const;

    void setField(Field field);
    void setSensorId(int sensorId);
    void setValue(double value);
    void setTolerance(double tolerance);
    void setValueOperation(ValueOperation operation);
    void setTimestampRange(qint64 fromMs, qint64 toMs);

    bool matches(const SensorData &record) const;
    QString toSqlCondition(const QString &tableAlias = QString()) const;

private:
    Field m_field = Field::SensorId;
    int m_sensorId = DEFAULT_SENSOR_ID;
    double m_value = DEFAULT_VALUE;
    double m_tolerance = DEFAULT_TOLERANCE;
    ValueOperation m_valueOperation = ValueOperation::Near;
    qint64 m_fromTimestampMs = DEFAULT_TIMESTAMP_MS;
    qint64 m_toTimestampMs = DEFAULT_TIMESTAMP_MS;
};

Q_DECLARE_METATYPE(FilterQuerySpec)

#endif // FILTERQUERYSPEC_H
