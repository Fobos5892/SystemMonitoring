#ifndef TELEMETRYMERGELOGIC_H
#define TELEMETRYMERGELOGIC_H

#include "Domain/filterqueryspec.h"
#include "Domain/sensordata.h"
#include "Domain/telemetrytypes.h"

#include <QVector>
#include <Qt>

namespace TelemetryMerge {

int compareBySort(const SensorData &lhs, const SensorData &rhs, int sortColumn, Qt::SortOrder sortOrder);

int findInsertIndex(const QVector<SensorData> &records, const SensorData &record, int sortColumn,
                    Qt::SortOrder sortOrder);

bool isWithinSortWindow(const SensorData &record, const SensorData &first, const SensorData &last,
                        int sortColumn, Qt::SortOrder sortOrder);

bool shouldAcceptAtViewportEdge(const SensorData &record, const QVector<SensorData> &records,
                                Telemetry::ViewportZone zone, int sortColumn,
                                Qt::SortOrder sortOrder);

bool containsRecordId(const QVector<SensorData> &records, quint64 recordId);

struct MergeOutcome {
    int insertedCount = 0;
    bool changed = false;
};

MergeOutcome mergeFilteredInsertions(QVector<SensorData> &records, const QVector<SensorData> &inserted,
                                     const FilterQuerySpec &filterSpec, int sortColumn,
                                     Qt::SortOrder sortOrder, int windowLimit, bool scrollIdle,
                                     Telemetry::ViewportZone viewportZone);

} // namespace TelemetryMerge

#endif // TELEMETRYMERGELOGIC_H
