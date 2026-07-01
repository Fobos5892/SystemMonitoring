#ifndef STATISTICSVIEWMODEL_H
#define STATISTICSVIEWMODEL_H

#include "Domain/sensorstatistics.h"
#include <QObject>
#include <QString>

class StatisticsViewModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString connectedLabel READ connectedLabel NOTIFY labelsChanged)
    Q_PROPERTY(QString averageLabel READ averageLabel NOTIFY labelsChanged)
    Q_PROPERTY(QString minimumLabel READ minimumLabel NOTIFY labelsChanged)
    Q_PROPERTY(QString maximumLabel READ maximumLabel NOTIFY labelsChanged)

public:
    explicit StatisticsViewModel(QObject *parent = nullptr);

    QString connectedLabel() const { return connectedLabelText; }
    QString averageLabel() const { return averageLabelText; }
    QString minimumLabel() const { return minimumLabelText; }
    QString maximumLabel() const { return maximumLabelText; }

public slots:
    void updateStatistics(const SensorStatistics &stats);

signals:
    void labelsChanged();

private:
    QString connectedLabelText;
    QString averageLabelText;
    QString minimumLabelText;
    QString maximumLabelText;
};

#endif // STATISTICSVIEWMODEL_H
