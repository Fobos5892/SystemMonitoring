#ifndef FILTERQUERYSPEC_H
#define FILTERQUERYSPEC_H

#include <QMetaType>
#include <QString>
#include <QtGlobal>

class FilterQuerySpec {
public:
    enum class Field : int {
        SensorId = 0,
        Value,
        Timestamp
    };

    enum class ValueOperation : int {
        Near = 0,
        Greater,
        Less
    };

    FilterQuerySpec() = default;

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

    QString toSqlCondition(const QString &tableAlias = QString()) const;

private:
    Field m_field = Field::SensorId;
    int m_sensorId = 0;
    double m_value = 0.0;
    double m_tolerance = 0.0;
    ValueOperation m_valueOperation = ValueOperation::Near;
    qint64 m_fromTimestampMs = 0;
    qint64 m_toTimestampMs = 0;
};

Q_DECLARE_METATYPE(FilterQuerySpec)

#endif // FILTERQUERYSPEC_H
