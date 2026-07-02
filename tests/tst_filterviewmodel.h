#ifndef TST_FILTERVIEWMODEL_H
#define TST_FILTERVIEWMODEL_H

#include <QObject>

class TestFilterViewModel : public QObject {
    Q_OBJECT

private slots:
    void normalizeValue_clampsAndTruncates();
    void normalizeTolerance_clampsAndTruncates();
    void adaptiveToleranceStep_selectsFineStepForSmallTolerance();
    void buildQuerySpec_sensorId();
    void buildQuerySpec_valueNear();
    void buildQuerySpec_valueGreater();
    void buildQuerySpec_valueLess();
    void buildQuerySpec_timestampRange_ordersBounds();
    void combineLocalDateTime_producesValidEpoch();
    void buildQuerySpec_timestampOnlyWhenFieldIsTimestamp();
};

#endif // TST_FILTERVIEWMODEL_H
