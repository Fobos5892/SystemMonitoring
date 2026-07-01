#ifndef TST_FILTERVIEWMODEL_H
#define TST_FILTERVIEWMODEL_H

#include <QObject>

class TestFilterViewModel : public QObject {
    Q_OBJECT

private slots:
    void normalizeValue_clampsAndTruncates();
    void normalizeTolerance_clampsAndTruncates();
    void adaptiveToleranceStep_selectsFineStepForSmallTolerance();
    void buildSqlCondition_sensorId();
    void buildSqlCondition_valueNear();
    void buildSqlCondition_valueGreater();
    void buildSqlCondition_valueLess();
    void buildSqlCondition_timestampRange_ordersBounds();
};

#endif // TST_FILTERVIEWMODEL_H
