#ifndef TELEMETRYVIEWMODEL_H
#define TELEMETRYVIEWMODEL_H

#include "Domain/sensordata.h"
#include "Domain/telemetrytypes.h"
#include <QObject>
#include <QScopedPointer>
#include <Qt>
#include <QVector>

class TelemetryTableModel;

class TelemetryViewModel : public QObject {
    Q_OBJECT

public:
    explicit TelemetryViewModel(QObject *parent = nullptr);
    ~TelemetryViewModel() override;

    TelemetryTableModel* tableModel() const { return table.data(); }

    int recordCount() const { return records.size(); }
    const SensorData& recordAt(int row) const { return records.at(row); }

    void setFollowLiveTail(bool follow);
    void setLiveUpdatesEnabled(bool enabled);
    void setFilterMode(bool enabled);
    bool isFollowingLiveTail() const { return followLiveTail; }
    bool isReloading() const { return reloading; }
    bool isLiveTailView() const;

    void handleRowAccessed(int row);

signals:
    void liveDataInserted();
    void loadingStarted();
    void loadingFinished();
    void sortRequested(int column, int sortOrder);
    void tailRequest(int sortColumn, int sortOrder, int limit);
    void rangeNearAnchorRequested(int sortColumn, int sortOrder, quint64 anchorRecordId,
                                  int limit, Telemetry::AnchorSide side);

public slots:
    void requestSort(int column, Qt::SortOrder order);
    void beginReloading();
    void onBatchCommitted();
    void onDataLoaded(const QVector<SensorData> &chunk);
    void onTailDataLoaded(const QVector<SensorData> &chunk);
    void onRangeNearAnchorLoaded(const QVector<SensorData> &chunk, Telemetry::AnchorSide side);
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

    QScopedPointer<TelemetryTableModel> table;

    QVector<SensorData> records;

    bool reachedTop = false;
    bool reachedBottom = false;
    bool bufferLoading = false;
    bool followLiveTail = true;
    bool liveUpdatesEnabled = true;
    bool filterMode = false;
    bool reloading = false;

    int currentSortColumn = static_cast<int>(Telemetry::Column::Timestamp);
    Qt::SortOrder currentSortOrder = Qt::AscendingOrder;
};

#endif // TELEMETRYVIEWMODEL_H
