#ifndef TST_SENSORSTATISTICS_H
#define TST_SENSORSTATISTICS_H

#include <QObject>

class TestSensorStatistics : public QObject {
    Q_OBJECT

private slots:
    void gettersReturnDefaults();
    void settersUpdateValues();
    void setConnectedCount_skipsDuplicate();
    void setDoubleFields_skipsNearDuplicate();
};

#endif // TST_SENSORSTATISTICS_H
