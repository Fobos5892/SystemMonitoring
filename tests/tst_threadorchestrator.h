#ifndef TST_THREADORCHESTRATOR_H
#define TST_THREADORCHESTRATOR_H

#include <QObject>

class TestThreadOrchestrator : public QObject {
    Q_OBJECT

private slots:
    void forwardsConnectionStatusFromSimulator();
};

#endif // TST_THREADORCHESTRATOR_H
