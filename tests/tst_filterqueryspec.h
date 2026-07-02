#ifndef TST_FILTERQUERYSPEC_H
#define TST_FILTERQUERYSPEC_H

#include <QObject>

class TestFilterQuerySpec : public QObject {
    Q_OBJECT

private slots:
    void gettersReturnDefaults();
    void settersUpdateValues();
    void setTimestampRange_ordersBounds();
    void toSqlCondition_sensorId();
    void toSqlCondition_valueNear();
    void toSqlCondition_valueGreater();
    void toSqlCondition_valueLess();
    void toSqlCondition_timestamp();
};

#endif // TST_FILTERQUERYSPEC_H
