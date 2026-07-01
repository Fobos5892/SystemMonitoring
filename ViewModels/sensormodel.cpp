#include "sensormodel.h"
#include "DBModel/dbdatacontroll.h"

SensorModel::SensorModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_dbController(nullptr)
    , m_reachedTop(false)
    , m_reachedBottom(false)
    , m_isBufferLoading(false)
    , m_followLiveTail(true)
    , m_liveUpdatesEnabled(true)
    , m_filterMode(false)
    , m_isReloading(false)
    , m_currentSortColumn(3)
    , m_currentSortOrder(Qt::AscendingOrder)
    , m_maxWindowSize(500)
    , m_chunkSize(40)
    , m_triggerThreshold(10)
{
}

void SensorModel::setDbController(DBDataControll* dbController) {
    m_dbController = dbController;
}

void SensorModel::setFollowLiveTail(bool follow) {
    m_followLiveTail = follow;
}

void SensorModel::setLiveUpdatesEnabled(bool enabled) {
    m_liveUpdatesEnabled = enabled;
    if (!enabled) {
        m_followLiveTail = false;
    }
}

void SensorModel::setFilterMode(bool enabled) {
    m_filterMode = enabled;
    if (enabled) {
        m_followLiveTail = false;
    }
}

bool SensorModel::isLiveTailView() const {
    return m_followLiveTail;
}

int SensorModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_records.size();
}

int SensorModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return 4;
}

QVariant SensorModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) {
        return QVariant();
    }
    if (m_isReloading || index.row() >= m_records.size()) {
        return QVariant();
    }

    const int currentRow = index.row();
    const int bufferSize = m_records.size();

    if (!m_filterMode && !m_isBufferLoading && !m_isReloading) {
        if (currentRow >= bufferSize - m_triggerThreshold) {
            const_cast<SensorModel*>(this)->slideWindowDown(m_chunkSize);
        } else if (currentRow <= m_triggerThreshold && bufferSize >= m_maxWindowSize) {
            const_cast<SensorModel*>(this)->slideWindowUp(m_chunkSize);
        }
    }

    const auto &record = m_records[currentRow];
    switch (index.column()) {
    case 0: return QVariant::fromValue(record.recordId);
    case 1: return QVariant::fromValue(record.sensorId);
    case 2: return record.value;
    case 3: return QLocale::system().toString(
                QDateTime::fromMSecsSinceEpoch(record.timestamp), "yyyy-MM-dd HH:mm:ss");
    }
    return QVariant();
}

QVariant SensorModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }
    switch (section) {
    case 0: return QStringLiteral("№ записи");
    case 1: return QStringLiteral("№датчика");
    case 2: return QStringLiteral("Значение");
    case 3: return QStringLiteral("Дата и Время");
    }
    return QVariant();
}

void SensorModel::beginReloading() {
    if (m_isReloading) {
        return;
    }

    m_isReloading = true;
    m_isBufferLoading = true;

    beginResetModel();
    m_records.clear();
    m_reachedTop = false;
    m_reachedBottom = false;
    endResetModel();

    emit loadingStarted();
}

void SensorModel::sort(int column, Qt::SortOrder order) {
    if (m_isReloading) {
        return;
    }

    if (!m_filterMode) {
        m_liveUpdatesEnabled = true;
    }
    m_currentSortColumn = column;
    m_currentSortOrder = order;

    beginReloading();
    emit sortRequested(column, static_cast<int>(order));
}

void SensorModel::finishReloading(const QVector<SensorData> &chunk) {
    m_records = chunk;
    m_reachedBottom = chunk.size() < m_maxWindowSize;
    m_reachedTop = chunk.size() < m_maxWindowSize;
    m_isBufferLoading = false;
    m_isReloading = false;
    m_followLiveTail = m_liveUpdatesEnabled;

    beginResetModel();
    endResetModel();

    emit loadingFinished();

    emit liveDataInserted();
}

void SensorModel::requestLiveRefresh() {
    if (m_isReloading) {
        return;
    }
    if (!m_liveUpdatesEnabled) {
        return;
    }

    const int sortOrder = static_cast<int>(m_currentSortOrder);

    if (m_records.isEmpty()) {
        emit tailWindowRequested(m_currentSortColumn, sortOrder, m_maxWindowSize);
        return;
    }

    if (!m_followLiveTail) {
        return;
    }

    if (m_currentSortOrder == Qt::AscendingOrder) {
        const quint64 anchorId = m_records.last().recordId;
        emit rangeAfterAnchorRequested(m_currentSortColumn, sortOrder, anchorId, m_chunkSize);
    } else {
        const quint64 anchorId = m_records.first().recordId;
        emit rangeBeforeAnchorRequested(m_currentSortColumn, sortOrder, anchorId, m_chunkSize);
    }
}

void SensorModel::onBatchCommitted() {
    if (!m_liveUpdatesEnabled) {
        return;
    }
    requestLiveRefresh();
}

void SensorModel::onReloadDataLoaded(const QVector<SensorData> &chunk) {
    if (!m_isReloading) {
        if (chunk.isEmpty()) {
            if (!m_records.isEmpty()) {
                beginResetModel();
                m_records.clear();
                m_reachedTop = true;
                m_reachedBottom = true;
                m_isBufferLoading = false;
                endResetModel();
            }
            return;
        }

        if (m_records.isEmpty()) {
            beginResetModel();
            m_records = chunk;
            m_reachedTop = chunk.size() < m_maxWindowSize;
            m_reachedBottom = chunk.size() < m_maxWindowSize;
            m_isBufferLoading = false;
            endResetModel();
            emit liveDataInserted();
            return;
        }

        // 1) Без изменений в диапазоне -> вообще не перерисовываем.
        if (m_records.size() == chunk.size()) {
            bool sameRange = true;
            for (int i = 0; i < chunk.size(); ++i) {
                if (m_records[i].recordId != chunk[i].recordId) {
                    sameRange = false;
                    break;
                }
            }
            if (sameRange) {
                return;
            }
        }

        // 2) Если пришли только новые записи в конец текущего диапазона.
        QVector<SensorData> appendChunk;
        appendChunk.reserve(chunk.size());
        for (const auto &record : chunk) {
            if (record.recordId > m_records.last().recordId) {
                appendChunk.append(record);
            }
        }
        if (!appendChunk.isEmpty()) {
            const int insertPos = m_records.size();
            beginInsertRows(QModelIndex(), insertPos, insertPos + appendChunk.size() - 1);
            m_records.append(appendChunk);
            endInsertRows();

            if (m_records.size() > m_maxWindowSize) {
                trimWindowFromTop(m_records.size() - m_maxWindowSize);
            }
            m_isBufferLoading = false;
            emit liveDataInserted();
            return;
        }

        // 3) Если пришли только новые записи в начало текущего диапазона.
        QVector<SensorData> prependChunk;
        prependChunk.reserve(chunk.size());
        for (const auto &record : chunk) {
            if (record.recordId < m_records.first().recordId) {
                prependChunk.append(record);
            }
        }
        if (!prependChunk.isEmpty()) {
            beginInsertRows(QModelIndex(), 0, prependChunk.size() - 1);
            for (int i = prependChunk.size() - 1; i >= 0; --i) {
                m_records.prepend(prependChunk[i]);
            }
            endInsertRows();

            if (m_records.size() > m_maxWindowSize) {
                trimWindowFromBottom(m_records.size() - m_maxWindowSize);
            }
            m_isBufferLoading = false;
            emit liveDataInserted();
            return;
        }

        // 4) Сложное изменение диапазона -> мягкий fallback.
        beginResetModel();
        m_records = chunk;
        m_reachedTop = chunk.size() < m_maxWindowSize;
        m_reachedBottom = chunk.size() < m_maxWindowSize;
        m_isBufferLoading = false;
        endResetModel();
        emit liveDataInserted();
        return;
    }
    finishReloading(chunk);
}

void SensorModel::onTailWindowLoaded(const QVector<SensorData> &chunk) {
    if (m_isReloading || chunk.isEmpty()) {
        return;
    }

    m_records = chunk;
    m_reachedTop = chunk.size() < m_maxWindowSize;
    m_reachedBottom = false;
    m_isBufferLoading = false;

    beginResetModel();
    endResetModel();

    emit liveDataInserted();
}

void SensorModel::trimWindowFromTop(int count) {
    if (count <= 0 || count > m_records.size()) {
        return;
    }
    beginRemoveRows(QModelIndex(), 0, count - 1);
    m_records.remove(0, count);
    endRemoveRows();
    m_reachedTop = false;
}

void SensorModel::trimWindowFromBottom(int count) {
    if (count <= 0 || count > m_records.size()) {
        return;
    }
    const int firstRemoved = m_records.size() - count;
    beginRemoveRows(QModelIndex(), firstRemoved, m_records.size() - 1);
    m_records.remove(firstRemoved, count);
    endRemoveRows();
    m_reachedBottom = false;
}

void SensorModel::onRangeAfterAnchorLoaded(const QVector<SensorData> &chunk) {
    if (m_isReloading) {
        return;
    }

    if (chunk.isEmpty()) {
        m_reachedBottom = true;
        m_isBufferLoading = false;
        return;
    }

    if (chunk.size() < m_chunkSize) {
        m_reachedBottom = true;
    }

    const int insertPos = m_records.size();
    beginInsertRows(QModelIndex(), insertPos, insertPos + chunk.size() - 1);
    m_records.append(chunk);
    endInsertRows();

    if (m_records.size() > m_maxWindowSize) {
        trimWindowFromTop(m_records.size() - m_maxWindowSize);
    }

    m_isBufferLoading = false;
    emit liveDataInserted();
}

void SensorModel::onRangeBeforeAnchorLoaded(const QVector<SensorData> &chunk) {
    if (m_isReloading) {
        return;
    }

    if (chunk.isEmpty()) {
        m_reachedTop = true;
        m_isBufferLoading = false;
        return;
    }

    if (chunk.size() < m_chunkSize) {
        m_reachedTop = true;
    }

    beginInsertRows(QModelIndex(), 0, chunk.size() - 1);
    for (int i = chunk.size() - 1; i >= 0; --i) {
        m_records.prepend(chunk[i]);
    }
    endInsertRows();

    if (m_records.size() > m_maxWindowSize) {
        trimWindowFromBottom(m_records.size() - m_maxWindowSize);
    }

    m_reachedBottom = false;
    m_isBufferLoading = false;
}

void SensorModel::slideWindowDown(int count) {
    if (m_filterMode || m_records.isEmpty() || m_reachedBottom || m_isReloading || m_isBufferLoading) {
        return;
    }

    m_isBufferLoading = true;
    const quint64 anchorId = m_records.last().recordId;
    emit rangeAfterAnchorRequested(m_currentSortColumn,
                                   static_cast<int>(m_currentSortOrder),
                                   anchorId,
                                   count);
}

void SensorModel::slideWindowUp(int count) {
    if (m_filterMode || m_records.isEmpty() || m_reachedTop || m_isReloading || m_isBufferLoading) {
        return;
    }

    m_isBufferLoading = true;
    const quint64 anchorId = m_records.first().recordId;
    emit rangeBeforeAnchorRequested(m_currentSortColumn,
                                    static_cast<int>(m_currentSortOrder),
                                    anchorId,
                                    count);
}

void SensorModel::onDatabaseCleared() {
    beginResetModel();
    m_records.clear();
    m_reachedTop = false;
    m_reachedBottom = false;
    m_isBufferLoading = false;
    m_isReloading = false;
    endResetModel();
    emit loadingFinished();
}
