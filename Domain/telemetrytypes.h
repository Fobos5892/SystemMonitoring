#ifndef TELEMETRYTYPES_H
#define TELEMETRYTYPES_H

#include <QObject>

namespace Telemetry {

Q_NAMESPACE

enum class AnchorSide {
    Top,
    Bottom
};
Q_ENUM_NS(AnchorSide)

enum class ConnectionStatus {
    Started,
    Stopped
};
Q_ENUM_NS(ConnectionStatus)

enum class ViewportZone {
    TopEdge,
    Middle,
    BottomEdge
};
Q_ENUM_NS(ViewportZone)

enum class Column : int {
    RecordId = 0,
    SensorId,
    Value,
    Timestamp
};

constexpr int WINDOW_SIZE = 500;
constexpr int CHUNK_SIZE = 40;
constexpr int TRIGGER_THRESHOLD = 10;
constexpr int MIN_REQUEST_LIMIT = 1; // At least one row to detect scroll boundaries.
constexpr int VOLTAGE_DISPLAY_DECIMAL_PLACES = 2;
constexpr int MS_PER_SECOND = 1000;
constexpr int SECONDS_PER_MINUTE = 60;
constexpr int MS_PER_MINUTE = SECONDS_PER_MINUTE * MS_PER_SECOND;
constexpr int SECONDS_PER_HOUR = 60 * SECONDS_PER_MINUTE;
constexpr qint64 HOUR_MS = static_cast<qint64>(SECONDS_PER_HOUR) * MS_PER_SECOND;
constexpr int SENSOR_ACTIVITY_WINDOW_MINUTES = 10;

} // namespace Telemetry

#endif // TELEMETRYTYPES_H
