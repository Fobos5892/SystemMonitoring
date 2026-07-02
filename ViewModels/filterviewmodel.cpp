#include "filterviewmodel.h"

#include <algorithm>
#include <cmath>
#include <QTimeZone>

FilterViewModel::FilterViewModel(QObject *parent)
    : QObject(parent)
    , dateFrom(QDateTime(QDate::currentDate(), QTime(0, 0), QTimeZone::systemTimeZone()))
    , dateTo(QDateTime::currentDateTime())
{
}

QDateTime FilterViewModel::combineLocalDateTime(const QDate &date, const QTime &time)
{
    return QDateTime(date, time, QTimeZone::systemTimeZone());
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
        spec.setTimestampRange(from.toMSecsSinceEpoch(), to.toMSecsSinceEpoch());
    }

    return spec;
}

