#ifndef TST_STATISTICSVIEWMODEL_H
#define TST_STATISTICSVIEWMODEL_H

#include <QObject>

class TestStatisticsViewModel : public QObject {
    Q_OBJECT

private slots:
    void updateStatistics_formatsLabels();
    void updateStatistics_skipsDuplicateEmit();
};

#endif // TST_STATISTICSVIEWMODEL_H
