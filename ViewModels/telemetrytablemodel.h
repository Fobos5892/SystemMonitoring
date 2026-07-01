#ifndef TELEMETRYTABLEMODEL_H
#define TELEMETRYTABLEMODEL_H

#include "Domain/telemetrytypes.h"
#include <QAbstractTableModel>
#include <QPointer>

class TelemetryViewModel;

class TelemetryTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit TelemetryTableModel(TelemetryViewModel *viewModel, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    void notifyFullReset();
    void notifyRowsInserted(int first, int last);
    void notifyRowsRemoved(int first, int last);

private:
    QPointer<TelemetryViewModel> viewModel;
};

#endif // TELEMETRYTABLEMODEL_H
