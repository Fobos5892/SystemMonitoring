#include "filterviewmodel.h"

#include <algorithm>
#include <cmath>
#include <QTimeZone>

FilterViewModel::FilterViewModel(QObject *parent)
    : QObject(parent)
    , dateFrom(combineLocalDateTime(QDate::currentDate(), DAY_START_TIME))
    , dateTo(combineLocalDateTime(QDate::currentDate(), QTime::currentTime(), true))
{
}

QDateTime FilterViewModel::combineLocalDateTime(const QDate &date, const QTime &time,
                                                bool inclusiveEnd)
{
    if (!date.isValid() || !time.isValid()) {
        return {};
    }

    QTime effectiveTime = time;
    if (inclusiveEnd) {
        effectiveTime = time.addSecs(INCLUSIVE_END_EXTRA_SECONDS)
                            .addMSecs(INCLUSIVE_END_EXTRA_MILLISECONDS);
    }

    const QString dateTimeText = QStringLiteral("%1 %2")
        .arg(date.toString(DatetimeFormats::DATE_PATTERN),
             effectiveTime.toString(DatetimeFormats::TIME_PATTERN));

    return parseLocalDateTimeText(dateTimeText);
}

QDateTime FilterViewModel::parseLocalDateTimeText(const QString &dateTimeText)
{
    QDateTime result = QDateTime::fromString(dateTimeText, DatetimeFormats::DATE_TIME_PATTERN);
    if (!result.isValid()) {
        return {};
    }
    result.setTimeZone(QTimeZone::systemTimeZone());
    return result;
}

qint64 FilterViewModel::epochMsFromDateTime(const QDateTime &dateTime)
{
    if (!dateTime.isValid()) {
        return QDateTime::currentMSecsSinceEpoch();
    }

    const QDateTime normalized =
        parseLocalDateTimeText(dateTime.toString(DatetimeFormats::DATE_TIME_PATTERN));
    return normalized.isValid() ? normalized.toMSecsSinceEpoch() : dateTime.toMSecsSinceEpoch();
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
    const double scale = std::pow(DECIMAL_RADIX, decimals);
    return std::floor(value * scale) / scale;
}

double FilterViewModel::normalizeValue(double value)
{
    return truncateTo(std::clamp(value, FilterLimits::MIN_SENSOR_VALUE_VOLTS,
                               FilterLimits::MAX_SENSOR_VALUE_VOLTS),
                      FilterLimits::SENSOR_VALUE_DECIMAL_PLACES);
}

double FilterViewModel::normalizeTolerance(double tolerance)
{
    return truncateTo(std::clamp(tolerance, FilterLimits::MIN_TOLERANCE, FilterLimits::MAX_TOLERANCE),
                      FilterLimits::TOLERANCE_DECIMAL_PLACES);
}

double FilterViewModel::adaptiveToleranceStep(double tolerance)
{
    return tolerance < FilterLimits::SMALL_TOLERANCE_THRESHOLD ? FilterLimits::FINE_TOLERANCE_STEP
                                                               : FilterLimits::COARSE_TOLERANCE_STEP;
}

FilterQuerySpec FilterViewModel::buildQuerySpec() const
{
    FilterQuerySpec spec;
    spec.setField(static_cast<FilterQuerySpec::Field>(field));
    spec.setSensorId(sensorId);
    spec.setValue(normalizeValue(value));
    spec.setTolerance(normalizeTolerance(tolerance));
    spec.setValueOperation(static_cast<FilterQuerySpec::ValueOperation>(valueOperation));

    if (field == Field::Timestamp) {
        const QDateTime from = dateFrom.isValid() ? dateFrom : QDateTime::currentDateTime();
        const QDateTime to = dateTo.isValid() ? dateTo : QDateTime::currentDateTime();
        spec.setTimestampRange(epochMsFromDateTime(from), epochMsFromDateTime(to));
    }

    return spec;
}
