#include "sensormodel.h"
#include "DBModel/dbdatacontroll.h"

#include <QLocale>
#include <QDebug>
#include <unordered_map>

namespace {

const std::unordered_map<int, QString> kColumnTitles = {
    {static_cast<int>(SensorModel::Column::RecordId), QStringLiteral("№ записи")},
    {static_cast<int>(SensorModel::Column::SensorId), QStringLiteral("№датчика")},
    {static_cast<int>(SensorModel::Column::Value), QStringLiteral("Значение")},
    {static_cast<int>(SensorModel::Column::Timestamp), QStringLiteral("Дата и Время")},
};

} // namespace

SensorModel::SensorModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void SensorModel::setDbController(DBDataControll* dbController) {
    if (dbController == nullptr) {
        qCritical() << "SensorModel: DBDataControll не может быть nullptr";
        return;
    }
    if (this->dbController == dbController) {
        return;
    }
    this->dbController = dbController;
}

void SensorModel::setFollowLiveTail(bool follow) {
    if (followLiveTail == follow) {
        return;
    }
    followLiveTail = follow;
}

void SensorModel::setLiveUpdatesEnabled(bool enabled) {
    if (liveUpdatesEnabled == enabled) {
        return;
    }
    liveUpdatesEnabled = enabled;
    if (!enabled) {
        followLiveTail = false;
    }
}

void SensorModel::setFilterMode(bool enabled) {
    if (filterMode == enabled) {
        return;
    }
    filterMode = enabled;
    if (enabled) {
        followLiveTail = false;
    }
}

bool SensorModel::isLiveTailView() const {
    return followLiveTail;
}

int SensorModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return records.size();
}

int SensorModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return DEFAULT_ROW_COLUMN_COUNT;
    }
    return static_cast<int>(kColumnTitles.size());
}

QVariant SensorModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    if (reloading || index.row() >= records.size()) {
        return QVariant();
    }

    const int currentRow = index.row();
    const int bufferSize = records.size();

    if (!filterMode && !bufferLoading && !reloading) {
        if (currentRow >= bufferSize - triggerThreshold) {
            const_cast<SensorModel*>(this)->slideWindow(chunkSize, SlideDirection::Down);
        } else if (currentRow <= triggerThreshold && bufferSize >= maxWindowSize) {
            const_cast<SensorModel*>(this)->slideWindow(chunkSize, SlideDirection::Up);
        }
    }

    const auto &record = records[currentRow];
    switch (static_cast<Column>(index.column())) {
    case Column::RecordId:
        return QVariant::fromValue(record.recordId);
    case Column::SensorId:
        return QVariant::fromValue(record.sensorId);
    case Column::Value:
        return record.value;
    case Column::Timestamp:
        return QLocale::system().toString(
            QDateTime::fromMSecsSinceEpoch(record.timestamp), "yyyy-MM-dd HH:mm:ss");
    }
    return QVariant();
}

QVariant SensorModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }
    const auto it = kColumnTitles.find(section);
    if (it != kColumnTitles.end()) {
        return it->second;
    }
    return QVariant();
}

void SensorModel::beginReloading() {
    if (reloading) {
        return;
    }

    reloading = true;
    bufferLoading = true;

    beginResetModel();
    records.clear();
    reachedTop = false;
    reachedBottom = false;
    endResetModel();

    emit loadingStarted();
}

void SensorModel::sort(int column, Qt::SortOrder order) {
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

void SensorModel::finishReloading(const QVector<SensorData> &chunk) {
    records = chunk;
    reachedBottom = chunk.size() < maxWindowSize;
    reachedTop = chunk.size() < maxWindowSize;
    bufferLoading = false;
    reloading = false;
    followLiveTail = liveUpdatesEnabled;

    beginResetModel();
    endResetModel();

    emit loadingFinished();

    emit liveDataInserted();
}

void SensorModel::requestLiveRefresh() {
    if (reloading) {
        return;
    }
    if (!liveUpdatesEnabled) {
        return;
    }

    const int sortOrder = static_cast<int>(currentSortOrder);

    if (records.isEmpty()) {
        emit tailRequest(currentSortColumn, sortOrder, maxWindowSize);
        return;
    }

    if (!followLiveTail) {
        return;
    }

    if (currentSortOrder == Qt::AscendingOrder) {
        const quint64 anchorId = records.last().recordId;
        emit rangeNearAnchorRequested(currentSortColumn, sortOrder, anchorId, chunkSize,
                                      DBDataControll::AnchorSide::Bottom);
    } else {
        const quint64 anchorId = records.first().recordId;
        emit rangeNearAnchorRequested(currentSortColumn, sortOrder, anchorId, chunkSize,
                                      DBDataControll::AnchorSide::Top);
    }
}

void SensorModel::onBatchCommitted() {
    if (!liveUpdatesEnabled) {
        return;
    }
    requestLiveRefresh();
}

void SensorModel::onDataLoaded(const QVector<SensorData> &chunk) {
    if (reloading) {
        finishReloading(chunk);
        return;
    }
    applyIncrementalDataUpdate(chunk);
}

void SensorModel::applyIncrementalDataUpdate(const QVector<SensorData> &chunk) {
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

void SensorModel::clearRecordsOnEmptyUpdate() {
    if (records.isEmpty()) {
        return;
    }

    beginResetModel();
    records.clear();
    reachedTop = true;
    reachedBottom = true;
    bufferLoading = false;
    endResetModel();
}

void SensorModel::populateInitialChunk(const QVector<SensorData> &chunk) {
    beginResetModel();
    records = chunk;
    setBoundaryFlagsForChunkSize(chunk.size());
    bufferLoading = false;
    endResetModel();
    emit liveDataInserted();
}

bool SensorModel::hasSameVisibleRange(const QVector<SensorData> &chunk) const {
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

bool SensorModel::tryAppendNewRecords(const QVector<SensorData> &chunk) {
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
    beginInsertRows(QModelIndex(), insertPos, insertPos + appendChunk.size() - 1);
    records.append(appendChunk);
    endInsertRows();

    if (records.size() > maxWindowSize) {
        trimWindowFromTop(records.size() - maxWindowSize);
    }
    bufferLoading = false;
    emit liveDataInserted();
    return true;
}

bool SensorModel::tryPrependNewRecords(const QVector<SensorData> &chunk) {
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

    beginInsertRows(QModelIndex(), 0, prependChunk.size() - 1);
    for (int i = prependChunk.size() - 1; i >= 0; --i) {
        records.prepend(prependChunk[i]);
    }
    endInsertRows();

    if (records.size() > maxWindowSize) {
        trimWindowFromBottom(records.size() - maxWindowSize);
    }
    bufferLoading = false;
    emit liveDataInserted();
    return true;
}

void SensorModel::resetRecordsFromChunk(const QVector<SensorData> &chunk) {
    beginResetModel();
    records = chunk;
    setBoundaryFlagsForChunkSize(chunk.size());
    bufferLoading = false;
    endResetModel();
    emit liveDataInserted();
}

void SensorModel::setBoundaryFlagsForChunkSize(int size) {
    reachedTop = size < maxWindowSize;
    reachedBottom = size < maxWindowSize;
}

void SensorModel::onTailDataLoaded(const QVector<SensorData> &chunk) {
    if (reloading || chunk.isEmpty()) {
        return;
    }

    records = chunk;
    reachedTop = chunk.size() < maxWindowSize;
    reachedBottom = false;
    bufferLoading = false;

    beginResetModel();
    endResetModel();

    emit liveDataInserted();
}

void SensorModel::trimWindowFromTop(int count) {
    if (count <= 0 || count > records.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, count - 1);
    records.remove(0, count);
    endRemoveRows();
    reachedTop = false;
}

void SensorModel::trimWindowFromBottom(int count) {
    if (count <= 0 || count > records.size()) {
        return;
    }
    const int firstRemoved = records.size() - count;
    beginRemoveRows(QModelIndex(), firstRemoved, records.size() - 1);
    records.remove(firstRemoved, count);
    endRemoveRows();
    reachedBottom = false;
}

void SensorModel::onRangeNearAnchorLoaded(const QVector<SensorData> &chunk,
                                          DBDataControll::AnchorSide side) {
    if (reloading) {
        return;
    }

    if (chunk.isEmpty()) {
        if (side == DBDataControll::AnchorSide::Bottom) {
            reachedBottom = true;
        } else {
            reachedTop = true;
        }
        bufferLoading = false;
        return;
    }

    if (chunk.size() < chunkSize) {
        if (side == DBDataControll::AnchorSide::Bottom) {
            reachedBottom = true;
        } else {
            reachedTop = true;
        }
    }

    if (side == DBDataControll::AnchorSide::Bottom) {
        const int insertPos = records.size();
        beginInsertRows(QModelIndex(), insertPos, insertPos + chunk.size() - 1);
        records.append(chunk);
        endInsertRows();

        if (records.size() > maxWindowSize) {
            trimWindowFromTop(records.size() - maxWindowSize);
        }

        bufferLoading = false;
        emit liveDataInserted();
        return;
    }

    beginInsertRows(QModelIndex(), 0, chunk.size() - 1);
    for (int i = chunk.size() - 1; i >= 0; --i) {
        records.prepend(chunk[i]);
    }
    endInsertRows();

    if (records.size() > maxWindowSize) {
        trimWindowFromBottom(records.size() - maxWindowSize);
    }

    reachedBottom = false;
    bufferLoading = false;
}

void SensorModel::slideWindow(int count, SlideDirection direction) {
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
    const auto side = slidingDown ? DBDataControll::AnchorSide::Bottom : DBDataControll::AnchorSide::Top;

    emit rangeNearAnchorRequested(currentSortColumn,
                                  static_cast<int>(currentSortOrder),
                                  anchorId,
                                  count,
                                  side);
}

void SensorModel::onDatabaseCleared() {
    beginResetModel();
    records.clear();
    reachedTop = false;
    reachedBottom = false;
    bufferLoading = false;
    reloading = false;
    endResetModel();
    emit loadingFinished();
}
