#ifndef TST_FILTERQUERYSPEC_H
#define TST_FILTERQUERYSPEC_H

#include <QObject>

class TestFilterQuerySpec : public QObject {
    Q_OBJECT

private slots:
    void gettersReturnDefaults();
    void settersUpdateValues();
    void setField_skipsDuplicate();
    void setSensorId_skipsDuplicate();
    void setValue_skipsNearDuplicate();
    void setTolerance_skipsNearDuplicate();
    void setValueOperation_skipsDuplicate();
    void setTimestampRange_ordersBounds();
    void setTimestampRange_skipsDuplicate();
    void toSqlCondition_sensorId();
    void toSqlCondition_valueNear();
    void toSqlCondition_valueGreater();
    void toSqlCondition_valueLess();
    void toSqlCondition_timestamp();
};

#endif // TST_FILTERQUERYSPEC_H
