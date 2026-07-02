#ifndef FILTERVIEWMODEL_H
#define FILTERVIEWMODEL_H

#include "Domain/filterqueryspec.h"

#include <QDate>
#include <QDateTime>
#include <QObject>
#include <QTime>

class FilterViewModel : public QObject {
    Q_OBJECT

public:
    enum class Field {
        SensorId = 0,
        Value,
        Timestamp
    };
    Q_ENUM(Field)

    enum class ValueOperation {
        Near = 0,
        Greater,
        Less
    };
    Q_ENUM(ValueOperation)

    explicit FilterViewModel(QObject *parent = nullptr);

    void setField(Field field);
    void setSensorId(int sensorId);
    void setValueFilter(double value, double tolerance, ValueOperation operation);
    void setTimestampRange(const QDateTime &from, const QDateTime &to);

    static QDateTime combineLocalDateTime(const QDate &date, const QTime &time);

    FilterQuerySpec buildQuerySpec() const;

    static double normalizeValue(double value);
    static double normalizeTolerance(double tolerance);
    static double adaptiveToleranceStep(double tolerance);

private:
    static double truncateTo(double value, int decimals);

    Field field = Field::SensorId;
    int sensorId = 0;
    double value = 0.0;
    double tolerance = 0.0;
    ValueOperation valueOperation = ValueOperation::Near;
    QDateTime dateFrom;
    QDateTime dateTo;
};

#endif // FILTERVIEWMODEL_H
