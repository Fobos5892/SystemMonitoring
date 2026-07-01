#include "telemetryviewmodel.h"
#include "telemetrytablemodel.h"

TelemetryViewModel::TelemetryViewModel(QObject *parent)
    : QObject(parent)
    , table(new TelemetryTableModel(this, this))
{
}

void TelemetryViewModel::setFollowLiveTail(bool follow)
{
    if (followLiveTail == follow) {
        return;
    }
    followLiveTail = follow;
}

void TelemetryViewModel::setLiveUpdatesEnabled(bool enabled)
{
    if (liveUpdatesEnabled == enabled) {
        return;
    }
    liveUpdatesEnabled = enabled;
    if (!enabled) {
        followLiveTail = false;
    }
}

void TelemetryViewModel::setFilterMode(bool enabled)
{
    if (filterMode == enabled) {
        return;
    }
    filterMode = enabled;
    if (enabled) {
        followLiveTail = false;
    }
}

bool TelemetryViewModel::isLiveTailView() const
{
    return followLiveTail;
}

void TelemetryViewModel::handleRowAccessed(int row)
{
    if (filterMode || bufferLoading || reloading) {
        return;
    }

    const int bufferSize = records.size();
    if (row >= bufferSize - Telemetry::TRIGGER_THRESHOLD) {
        slideWindow(Telemetry::CHUNK_SIZE, SlideDirection::Down);
    } else if (row <= Telemetry::TRIGGER_THRESHOLD && bufferSize >= Telemetry::WINDOW_SIZE) {
        slideWindow(Telemetry::CHUNK_SIZE, SlideDirection::Up);
    }
}

void TelemetryViewModel::beginReloading()
{
    if (reloading) {
        return;
    }

    reloading = true;
    bufferLoading = true;

    records.clear();
    reachedTop = false;
    reachedBottom = false;
    table->notifyFullReset();

    emit loadingStarted();
}

void TelemetryViewModel::requestSort(int column, Qt::SortOrder order)
{
    if (reloading) {
        return;
    }

    if (!filterMode) {
        liveUpdatesEnabled = true;
    }
    currentSortColumn = column;
    currentSortOrder = order;

    beginReloading();
    emit sortRequested(column, static_cast<int>(order));
}

void TelemetryViewModel::finishReloading(const QVector<SensorData> &chunk)
{
    records = chunk;
    reachedBottom = chunk.size() < Telemetry::WINDOW_SIZE;
    reachedTop = chunk.size() < Telemetry::WINDOW_SIZE;
    bufferLoading = false;
    reloading = false;
    followLiveTail = liveUpdatesEnabled;

    table->notifyFullReset();

    emit loadingFinished();
    emit liveDataInserted();
}

void TelemetryViewModel::requestLiveRefresh()
{
    if (reloading) {
        return;
    }
    if (!liveUpdatesEnabled) {
        return;
    }

    const int sortOrder = static_cast<int>(currentSortOrder);

    if (records.isEmpty()) {
        emit tailRequest(currentSortColumn, sortOrder, Telemetry::WINDOW_SIZE);
        return;
    }

    if (!followLiveTail) {
        return;
    }

    if (currentSortOrder == Qt::AscendingOrder) {
        const quint64 anchorId = records.last().recordId;
        emit rangeNearAnchorRequested(currentSortColumn, sortOrder, anchorId, Telemetry::CHUNK_SIZE,
                                      Telemetry::AnchorSide::Bottom);
    } else {
        const quint64 anchorId = records.first().recordId;
        emit rangeNearAnchorRequested(currentSortColumn, sortOrder, anchorId, Telemetry::CHUNK_SIZE,
                                      Telemetry::AnchorSide::Top);
    }
}

void TelemetryViewModel::onBatchCommitted()
{
    if (!liveUpdatesEnabled) {
        return;
    }
    requestLiveRefresh();
}

void TelemetryViewModel::onDataLoaded(const QVector<SensorData> &chunk)
{
    if (reloading) {
        finishReloading(chunk);
        return;
    }
    applyIncrementalDataUpdate(chunk);
}

void TelemetryViewModel::applyIncrementalDataUpdate(const QVector<SensorData> &chunk)
{
    if (chunk.isEmpty()) {
        clearRecordsOnEmptyUpdate();
        return;
    }
    if (records.isEmpty()) {
        populateInitialChunk(chunk);
        return;
    }
    if (hasSameVisibleRange(chunk)) {
        return;
    }
    if (tryAppendNewRecords(chunk)) {
        return;
    }
    if (tryPrependNewRecords(chunk)) {
        return;
    }
    resetRecordsFromChunk(chunk);
}

void TelemetryViewModel::clearRecordsOnEmptyUpdate()
{
    if (records.isEmpty()) {
        return;
    }

    records.clear();
    reachedTop = true;
    reachedBottom = true;
    bufferLoading = false;
    table->notifyFullReset();
}

void TelemetryViewModel::populateInitialChunk(const QVector<SensorData> &chunk)
{
    records = chunk;
    setBoundaryFlagsForChunkSize(chunk.size());
    bufferLoading = false;
    table->notifyFullReset();
    emit liveDataInserted();
}

bool TelemetryViewModel::hasSameVisibleRange(const QVector<SensorData> &chunk) const
{
    if (records.size() != chunk.size()) {
        return false;
    }
    for (int i = 0; i < chunk.size(); ++i) {
        if (records[i].recordId != chunk[i].recordId) {
            return false;
        }
    }
    return true;
}

bool TelemetryViewModel::tryAppendNewRecords(const QVector<SensorData> &chunk)
{
    QVector<SensorData> appendChunk;
    appendChunk.reserve(chunk.size());
    for (const auto &record : chunk) {
        if (record.recordId > records.last().recordId) {
            appendChunk.append(record);
        }
    }
    if (appendChunk.isEmpty()) {
        return false;
    }

    const int insertPos = records.size();
    records.append(appendChunk);
    table->notifyRowsInserted(insertPos, insertPos + appendChunk.size() - 1);

    if (records.size() > Telemetry::WINDOW_SIZE) {
        trimWindowFromTop(records.size() - Telemetry::WINDOW_SIZE);
    }
    bufferLoading = false;
    emit liveDataInserted();
    return true;
}

bool TelemetryViewModel::tryPrependNewRecords(const QVector<SensorData> &chunk)
{
    QVector<SensorData> prependChunk;
    prependChunk.reserve(chunk.size());
    for (const auto &record : chunk) {
        if (record.recordId < records.first().recordId) {
            prependChunk.append(record);
        }
    }
    if (prependChunk.isEmpty()) {
        return false;
    }

    for (int i = prependChunk.size() - 1; i >= 0; --i) {
        records.prepend(prependChunk[i]);
    }
    table->notifyRowsInserted(0, prependChunk.size() - 1);

    if (records.size() > Telemetry::WINDOW_SIZE) {
        trimWindowFromBottom(records.size() - Telemetry::WINDOW_SIZE);
    }
    bufferLoading = false;
    emit liveDataInserted();
    return true;
}

void TelemetryViewModel::resetRecordsFromChunk(const QVector<SensorData> &chunk)
{
    records = chunk;
    setBoundaryFlagsForChunkSize(chunk.size());
    bufferLoading = false;
    table->notifyFullReset();
    emit liveDataInserted();
}

void TelemetryViewModel::setBoundaryFlagsForChunkSize(int size)
{
    reachedTop = size < Telemetry::WINDOW_SIZE;
    reachedBottom = size < Telemetry::WINDOW_SIZE;
}

void TelemetryViewModel::onTailDataLoaded(const QVector<SensorData> &chunk)
{
    if (reloading || chunk.isEmpty()) {
        return;
    }

    records = chunk;
    reachedTop = chunk.size() < Telemetry::WINDOW_SIZE;
    reachedBottom = false;
    bufferLoading = false;

    table->notifyFullReset();
    emit liveDataInserted();
}

void TelemetryViewModel::trimWindowFromTop(int count)
{
    if (count <= 0 || count > records.size()) {
        return;
    }
    records.remove(0, count);
    table->notifyRowsRemoved(0, count - 1);
    reachedTop = false;
}

void TelemetryViewModel::trimWindowFromBottom(int count)
{
    if (count <= 0 || count > records.size()) {
        return;
    }
    const int firstRemoved = records.size() - count;
    const int lastRemoved = records.size() - 1;
    table->notifyRowsRemoved(firstRemoved, lastRemoved);
    records.remove(firstRemoved, count);
    reachedBottom = false;
}

void TelemetryViewModel::onRangeNearAnchorLoaded(const QVector<SensorData> &chunk,
                                                 Telemetry::AnchorSide side)
{
    if (reloading) {
        return;
    }

    if (chunk.isEmpty()) {
        if (side == Telemetry::AnchorSide::Bottom) {
            reachedBottom = true;
        } else {
            reachedTop = true;
        }
        bufferLoading = false;
        return;
    }

    if (chunk.size() < Telemetry::CHUNK_SIZE) {
        if (side == Telemetry::AnchorSide::Bottom) {
            reachedBottom = true;
        } else {
            reachedTop = true;
        }
    }

    if (side == Telemetry::AnchorSide::Bottom) {
        const int insertPos = records.size();
        records.append(chunk);
        table->notifyRowsInserted(insertPos, insertPos + chunk.size() - 1);

        if (records.size() > Telemetry::WINDOW_SIZE) {
            trimWindowFromTop(records.size() - Telemetry::WINDOW_SIZE);
        }

        bufferLoading = false;
        emit liveDataInserted();
        return;
    }

    for (int i = chunk.size() - 1; i >= 0; --i) {
        records.prepend(chunk[i]);
    }
    table->notifyRowsInserted(0, chunk.size() - 1);

    if (records.size() > Telemetry::WINDOW_SIZE) {
        trimWindowFromBottom(records.size() - Telemetry::WINDOW_SIZE);
    }

    reachedBottom = false;
    bufferLoading = false;
}

void TelemetryViewModel::slideWindow(int count, SlideDirection direction)
{
    const bool slidingDown = direction == SlideDirection::Down;

    if (filterMode || records.isEmpty() || reloading || bufferLoading) {
        return;
    }
    if (slidingDown && reachedBottom) {
        return;
    }
    if (!slidingDown && reachedTop) {
        return;
    }

    bufferLoading = true;
    const quint64 anchorId = slidingDown ? records.last().recordId : records.first().recordId;
    const auto side = slidingDown ? Telemetry::AnchorSide::Bottom : Telemetry::AnchorSide::Top;

    emit rangeNearAnchorRequested(currentSortColumn,
                                  static_cast<int>(currentSortOrder),
                                  anchorId,
                                  count,
                                  side);
}

void TelemetryViewModel::onDatabaseCleared()
{
    records.clear();
    reachedTop = false;
    reachedBottom = false;
    bufferLoading = false;
    reloading = false;
    table->notifyFullReset();
    emit loadingFinished();
}
