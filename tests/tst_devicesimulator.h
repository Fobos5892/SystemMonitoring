#ifndef TST_DEVICESIMULATOR_H
#define TST_DEVICESIMULATOR_H

#include <QObject>

class TestDeviceSimulator : public QObject {
    Q_OBJECT

private slots:
    void startGeneration_emitsStartedStatus();
    void stopGeneration_emitsStoppedStatus();
    void startGeneration_whenAlreadyStarted_doesNotEmitAgain();
    void stopGeneration_whenNotStarted_emitsStoppedStatus();
    void startThenStop_emitsStartedThenStopped();
    void startGeneration_producesDataBatch();
};

#endif // TST_DEVICESIMULATOR_H
