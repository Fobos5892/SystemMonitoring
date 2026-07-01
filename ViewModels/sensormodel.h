#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include "Data/sensordata.h"
#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>

class DBDataControll;

class SensorModel : public QAbstractTableModel {
    Q_OBJECT
public:
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
    bool isFollowingLiveTail() const { return m_followLiveTail; }
    bool isReloading() const { return m_isReloading; }
    bool isLiveTailView() const;

signals:
    void liveDataInserted();
    void loadingStarted();
    void loadingFinished();
    void sortRequested(int column, int sortOrder);
    void tailWindowRequested(int sortColumn, int sortOrder, int limit);
    void rangeAfterAnchorRequested(int sortColumn, int sortOrder, quint64 anchorRecordId, int limit);
    void rangeBeforeAnchorRequested(int sortColumn, int sortOrder, quint64 anchorRecordId, int limit);

public slots:
    void beginReloading();
    void onBatchCommitted();
    void onReloadDataLoaded(const QVector<SensorData> &chunk);
    void onTailWindowLoaded(const QVector<SensorData> &chunk);
    void onRangeAfterAnchorLoaded(const QVector<SensorData> &chunk);
    void onRangeBeforeAnchorLoaded(const QVector<SensorData> &chunk);
    void onDatabaseCleared();

private:
    void finishReloading(const QVector<SensorData> &chunk);
    void requestLiveRefresh();
    void slideWindowDown(int count);
    void slideWindowUp(int count);
    void trimWindowFromTop(int count);
    void trimWindowFromBottom(int count);

    QVector<SensorData> m_records;
    DBDataControll* m_dbController;

    bool m_reachedTop;
    bool m_reachedBottom;
    bool m_isBufferLoading;
    bool m_followLiveTail;
    bool m_liveUpdatesEnabled;
    bool m_filterMode;
    bool m_isReloading;

    int m_currentSortColumn;
    Qt::SortOrder m_currentSortOrder;

    const int m_maxWindowSize;
    const int m_chunkSize;
    const int m_triggerThreshold;
};

#endif // SENSORMODEL_H
