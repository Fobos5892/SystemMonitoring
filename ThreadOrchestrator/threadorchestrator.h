#ifndef THREADORCHESTRATOR_H
#define THREADORCHESTRATOR_H

#include <QObject>
#include <QThread>
#include <QScopedPointer>
#include <QString>

// Опережающее объявление классов, чтобы хидер оставался ультра-легким
class DeviceSimulator;
class DeviceReceiver;
class DBDataControll;
class SensorModel;

class ThreadOrchestrator : public QObject {
    Q_OBJECT
public:
    explicit ThreadOrchestrator(SensorModel* uiModel, QObject *parent = nullptr);
    ~ThreadOrchestrator();

    // Управление жизненным циклом потоков
    void startAll();
    void stopAll();

public slots:
    // Слоты для команд от интерфейса (MainWindow)
    void onConnectRequested();
    void onFilterRequested(const QString &filterCondition);

private:
    void initThreads();
    void setupConnections();

    // Указатель на интерфейсную модель (ViewModel)
    SensorModel* m_uiModel;

    // Умные указатели Qt на фоновые компоненты бизнес-логики
    QScopedPointer<DeviceSimulator> m_simulator;
    QScopedPointer<DeviceReceiver>  m_receiver;
    QScopedPointer<DBDataControll>  m_dbController;

    // Фоновые потоки
    QThread m_simulatorThread;
    QThread m_receiverThread;
    QThread m_sqlThread;
};
#endif // THREADORCHESTRATOR_H
