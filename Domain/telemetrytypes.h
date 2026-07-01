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

enum class Column : int {
    RecordId = 0,
    SensorId,
    Value,
    Timestamp
};

constexpr int WINDOW_SIZE = 500;
constexpr int CHUNK_SIZE = 40;
constexpr int TRIGGER_THRESHOLD = 10;
constexpr int DEFAULT_CHILD_COLUMN_COUNT = 0;

} // namespace Telemetry

#endif // TELEMETRYTYPES_H
