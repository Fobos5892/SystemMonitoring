#ifndef DATETIMEFORMATS_H
#define DATETIMEFORMATS_H

#include <QString>

namespace DatetimeFormats {

static constexpr int DAY_START_HOUR = 0;
static constexpr int DAY_START_MINUTE = 0;

static inline const QString DATE_PATTERN = QStringLiteral("yyyy-MM-dd");
static inline const QString TIME_PATTERN = QStringLiteral("HH:mm:ss");
static inline const QString DATE_TIME_PATTERN = QStringLiteral("yyyy-MM-dd HH:mm:ss");

} // namespace DatetimeFormats

#endif // DATETIMEFORMATS_H
