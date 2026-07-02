#include "telemetrytablemodel.h"
#include "telemetryviewmodel.h"
#include "Domain/datetimeformats.h"

#include <QDateTime>
#include <QLocale>
#include <unordered_map>

namespace {

const std::unordered_map<int, QString> COLUMN_TITLES = {
    {static_cast<int>(Telemetry::Column::RecordId), QStringLiteral("№ записи")},
    {static_cast<int>(Telemetry::Column::SensorId), QStringLiteral("№датчика")},
    {static_cast<int>(Telemetry::Column::Value), QStringLiteral("Значение")},
    {static_cast<int>(Telemetry::Column::Timestamp), QStringLiteral("Дата и Время")},
};

} // namespace

TelemetryTableModel::TelemetryTableModel(TelemetryViewModel *viewModel, QObject *parent)
    : QAbstractTableModel(parent)
    , viewModel(viewModel)
{
}

int TelemetryTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || viewModel == nullptr) {
        return 0;
    }
    return viewModel->recordCount();
}

int TelemetryTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(COLUMN_TITLES.size());
}

QVariant TelemetryTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole || viewModel == nullptr) {
        return QVariant();
    }
    if (viewModel->isReloading() || index.row() >= viewModel->recordCount()) {
        return QVariant();
    }

    viewModel->handleRowAccessed(index.row());

    const SensorData &record = viewModel->recordAt(index.row());
    switch (static_cast<Telemetry::Column>(index.column())) {
    case Telemetry::Column::RecordId:
        return QVariant::fromValue(record.recordId);
    case Telemetry::Column::SensorId:
        return QVariant::fromValue(record.sensorId);
    case Telemetry::Column::Value:
        return record.value;
    case Telemetry::Column::Timestamp:
        return QLocale::system().toString(QDateTime::fromMSecsSinceEpoch(record.timestamp),
                                          DatetimeFormats::DATE_TIME_PATTERN);
    }
    return QVariant();
}

QVariant TelemetryTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }
    const auto it = COLUMN_TITLES.find(section);
    if (it != COLUMN_TITLES.end()) {
        return it->second;
    }
    return QVariant();
}

void TelemetryTableModel::sort(int column, Qt::SortOrder order)
{
    if (viewModel != nullptr) {
        viewModel->requestSort(column, order);
    }
}

void TelemetryTableModel::notifyFullReset()
{
    beginResetModel();
    endResetModel();
}

void TelemetryTableModel::notifyRowsInserted(int first, int last)
{
    beginInsertRows(QModelIndex(), first, last);
    endInsertRows();
}

void TelemetryTableModel::notifyRowsRemoved(int first, int last)
{
    beginRemoveRows(QModelIndex(), first, last);
    endRemoveRows();
}
