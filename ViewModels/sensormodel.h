#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include "Data/sensordata.h"
#include "DBModel/dbdatacontroll.h"
#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>

class DBDataControll;

class SensorModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum class Column : int {
        RecordId = 0,
        SensorId,
        Value,
        Timestamp
    };

    static constexpr int DEFAULT_ROW_COLUMN_COUNT = 0;

    explicit SensorModel(QObject *parent = nullptr);
    ~SensorModel() = default;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void setDbController(DBDataControll* dbController);

    void setFollowLiveTail(bool follow);
    void setLiveUpdatesEnabled(bool enabled);
    void setFilterMode(bool enabled);
    bool isFollowingLiveTail() const { return followLiveTail; }
    bool isReloading() const { return reloading; }
    bool isLiveTailView() const;

signals:
    void liveDataInserted();
    void loadingStarted();
    void loadingFinished();
    void sortRequested(int column, int sortOrder);
    void tailRequest(int sortColumn, int sortOrder, int limit);
    void rangeNearAnchorRequested(int sortColumn, int sortOrder, quint64 anchorRecordId,
                                  int limit, DBDataControll::AnchorSide side);

public slots:
    void beginReloading();
    void onBatchCommitted();
    void onDataLoaded(const QVector<SensorData> &chunk);
    void onTailDataLoaded(const QVector<SensorData> &chunk);
    void onRangeNearAnchorLoaded(const QVector<SensorData> &chunk, DBDataControll::AnchorSide side);
    void onDatabaseCleared();

private:
    enum class SlideDirection {
        Down,
        Up
    };

    void finishReloading(const QVector<SensorData> &chunk);
    void requestLiveRefresh();
    void applyIncrementalDataUpdate(const QVector<SensorData> &chunk);
    void clearRecordsOnEmptyUpdate();
    void populateInitialChunk(const QVector<SensorData> &chunk);
    bool hasSameVisibleRange(const QVector<SensorData> &chunk) const;
    bool tryAppendNewRecords(const QVector<SensorData> &chunk);
    bool tryPrependNewRecords(const QVector<SensorData> &chunk);
    void resetRecordsFromChunk(const QVector<SensorData> &chunk);
    void setBoundaryFlagsForChunkSize(int size);
    void slideWindow(int count, SlideDirection direction);
    void trimWindowFromTop(int count);
    void trimWindowFromBottom(int count);

    QVector<SensorData> records;
    DBDataControll* dbController = nullptr;

    bool reachedTop = false;
    bool reachedBottom = false;
    bool bufferLoading = false;
    bool followLiveTail = true;
    bool liveUpdatesEnabled = true;
    bool filterMode = false;
    bool reloading = false;

    int currentSortColumn = static_cast<int>(Column::Timestamp);
    Qt::SortOrder currentSortOrder = Qt::AscendingOrder;

    const int maxWindowSize = 500;
    const int chunkSize = 40;
    const int triggerThreshold = 10;
};

#endif // SENSORMODEL_H
