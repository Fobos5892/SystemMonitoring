#ifndef TST_SENSORSTATISTICS_H
#define TST_SENSORSTATISTICS_H

#include <QObject>

class TestSensorStatistics : public QObject {
    Q_OBJECT

private slots:
    void gettersReturnDefaults();
    void settersUpdateValues();
};

#endif // TST_SENSORSTATISTICS_H
