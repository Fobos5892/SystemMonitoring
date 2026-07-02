#include "telemetrymergelogic.h"

#include <algorithm>

namespace TelemetryMerge {

namespace {

double sortableValue(const SensorData &record, int sortColumn)
{
    switch (static_cast<Telemetry::Column>(sortColumn)) {
    case Telemetry::Column::RecordId:
        return static_cast<double>(record.recordId);
    case Telemetry::Column::SensorId:
        return static_cast<double>(record.sensorId);
    case Telemetry::Column::Value:
        return record.value;
    case Telemetry::Column::Timestamp:
        return static_cast<double>(record.timestamp);
    }
    return static_cast<double>(record.timestamp);
}

} // namespace

int compareBySort(const SensorData &lhs, const SensorData &rhs, int sortColumn, Qt::SortOrder sortOrder)
{
    const double left = sortableValue(lhs, sortColumn);
    const double right = sortableValue(rhs, sortColumn);
    if (left < right) {
        return sortOrder == Qt::AscendingOrder ? -1 : 1;
    }
    if (left > right) {
        return sortOrder == Qt::AscendingOrder ? 1 : -1;
    }

    if (lhs.recordId < rhs.recordId) {
        return sortOrder == Qt::AscendingOrder ? -1 : 1;
    }
    if (lhs.recordId > rhs.recordId) {
        return sortOrder == Qt::AscendingOrder ? 1 : -1;
    }
    return 0;
}

int findInsertIndex(const QVector<SensorData> &records, const SensorData &record, int sortColumn,
                    Qt::SortOrder sortOrder)
{
    int left = 0;
    int right = records.size();
    while (left < right) {
        const int mid = left + (right - left) / 2;
        if (compareBySort(record, records.at(mid), sortColumn, sortOrder) > 0) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

bool isWithinSortWindow(const SensorData &record, const SensorData &first, const SensorData &last,
                        int sortColumn, Qt::SortOrder sortOrder)
{
    return compareBySort(record, first, sortColumn, sortOrder) >= 0
        && compareBySort(record, last, sortColumn, sortOrder) <= 0;
}

bool shouldAcceptAtViewportEdge(const SensorData &record, const QVector<SensorData> &records,
                                Telemetry::ViewportZone zone, int sortColumn, Qt::SortOrder sortOrder)
{
    if (records.isEmpty()) {
        return true;
    }

    const SensorData &first = records.first();
    const SensorData &last = records.last();

    switch (zone) {
    case Telemetry::ViewportZone::TopEdge:
        return compareBySort(record, first, sortColumn, sortOrder) <= 0;
    case Telemetry::ViewportZone::BottomEdge:
        return compareBySort(record, last, sortColumn, sortOrder) >= 0;
    case Telemetry::ViewportZone::Middle:
        return isWithinSortWindow(record, first, last, sortColumn, sortOrder);
    }
    return false;
}

bool containsRecordId(const QVector<SensorData> &records, quint64 recordId)
{
    for (const SensorData &record : records) {
        if (record.recordId == recordId) {
            return true;
        }
    }
    return false;
}

void trimToLimit(QVector<SensorData> &records, int windowLimit, Telemetry::ViewportZone viewportZone)
{
    while (records.size() > windowLimit) {
        if (viewportZone == Telemetry::ViewportZone::TopEdge) {
            records.removeLast();
            continue;
        }
        records.removeFirst();
    }
}

} // namespace TelemetryMerge

namespace TelemetryMerge {

MergeOutcome mergeFilteredInsertions(QVector<SensorData> &records, const QVector<SensorData> &inserted,
                                     const FilterQuerySpec &filterSpec, int sortColumn,
                                     Qt::SortOrder sortOrder, int windowLimit, bool scrollIdle,
                                     Telemetry::ViewportZone viewportZone)
{
    MergeOutcome outcome;
    if (!scrollIdle || inserted.isEmpty() || windowLimit < Telemetry::MIN_REQUEST_LIMIT) {
        return outcome;
    }

    QVector<SensorData> accepted;
    accepted.reserve(inserted.size());
    for (const SensorData &record : inserted) {
        if (record.recordId == SensorData::DEFAULT_RECORD_ID) {
            continue;
        }
        if (!filterSpec.matches(record)) {
            continue;
        }
        if (containsRecordId(records, record.recordId) || containsRecordId(accepted, record.recordId)) {
            continue;
        }
        if (!records.isEmpty()
            && !shouldAcceptAtViewportEdge(record, records, viewportZone, sortColumn, sortOrder)) {
            continue;
        }
        accepted.append(record);
    }

    if (accepted.isEmpty()) {
        return outcome;
    }

    std::sort(accepted.begin(), accepted.end(),
              [sortColumn, sortOrder](const SensorData &lhs, const SensorData &rhs) {
                  return compareBySort(lhs, rhs, sortColumn, sortOrder) < 0;
              });

    for (const SensorData &record : accepted) {
        const int insertIndex = findInsertIndex(records, record, sortColumn, sortOrder);
        records.insert(insertIndex, record);
        ++outcome.insertedCount;
    }

    trimToLimit(records, windowLimit, viewportZone);
    outcome.changed = outcome.insertedCount > 0;
    return outcome;
}

} // namespace TelemetryMerge
