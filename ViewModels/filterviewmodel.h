#ifndef FILTERVIEWMODEL_H
#define FILTERVIEWMODEL_H

#include "Domain/datetimeformats.h"
#include "Domain/filterlimits.h"
#include "Domain/filterqueryspec.h"

#include <QDate>
#include <QDateTime>
#include <QObject>
#include <QString>
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

    static QDateTime combineLocalDateTime(const QDate &date, const QTime &time,
                                          bool inclusiveEnd = false);

    static inline const QTime DAY_START_TIME{
        QTime(DatetimeFormats::DAY_START_HOUR, DatetimeFormats::DAY_START_MINUTE)};

    FilterQuerySpec buildQuerySpec() const;

    static double normalizeValue(double value);
    static double normalizeTolerance(double tolerance);
    static double adaptiveToleranceStep(double tolerance);

private:
    static constexpr double DECIMAL_RADIX = 10.0;

    static constexpr int INCLUSIVE_END_EXTRA_SECONDS = 59;
    static constexpr int INCLUSIVE_END_EXTRA_MILLISECONDS = 999;

    static double truncateTo(double value, int decimals);
    static qint64 epochMsFromDateTime(const QDateTime &dateTime);
    static QDateTime parseLocalDateTimeText(const QString &dateTimeText);

    Field field = Field::SensorId;
    int sensorId = FilterQuerySpec::DEFAULT_SENSOR_ID;
    double value = FilterQuerySpec::DEFAULT_VALUE;
    double tolerance = FilterQuerySpec::DEFAULT_TOLERANCE;
    ValueOperation valueOperation = ValueOperation::Near;
    QDateTime dateFrom;
    QDateTime dateTo;
};

#endif // FILTERVIEWMODEL_H
