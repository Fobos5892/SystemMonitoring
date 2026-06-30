#include "threadorchestrator.h"
#include "DeviceSimulator/devicesimulator.h"
#include "DeviceReceiver/devicereceiver.h"
#include "DBModel/dbdatacontroll.h"
#include "ViewModels/sensormodel.h"

ThreadOrchestrator::ThreadOrchestrator(SensorModel* uiModel, QObject *parent)
    : QObject(parent)
    , m_uiModel(uiModel)
    // Выделяем память через обычный new, QScopedPointer забирает владение объектами
    , m_simulator(new DeviceSimulator())
    , m_receiver(new DeviceReceiver())
    , m_dbController(new DBDataControll("telemetry.db"))
{
    initThreads();
    setupConnections();
}

ThreadOrchestrator::~ThreadOrchestrator() {
    stopAll(); // Гарантируем безопасное завершение потоков при деструкции
}

void ThreadOrchestrator::initThreads() {
    // Переносим объекты в их персональные фоновые потоки.
    // Используем .data() для получения сырого указателя из QScopedPointer
    m_simulator->moveToThread(&m_simulatorThread);
    m_receiver->moveToThread(&m_receiverThread);
    m_dbController->moveToThread(&m_sqlThread);
}

void ThreadOrchestrator::setupConnections() {
    // Шаг 1: Фоновый конвейер данных (Генератор -> Ресивер -> БД и UI)
    // connect(m_simulator.data(), &DeviceSimulator::rawDataGenerated,
    //         m_receiver.data(), &DeviceReceiver::onRawDataReceived);

    // connect(m_receiver.data(), &DeviceReceiver::dataBatchReady,
    //         m_dbController.data(), &DBDataControll::onSaveBatchToSql);

    // connect(m_receiver.data(), &DeviceReceiver::dataBatchReady,
    //         m_uiModel, &SensorModel::onLiveRecordsReceived);

    // // Шаг 2: Обратная связь БД -> UI модель при скроллинге истории (вызовы fetch)
    // connect(m_dbController.data(), &DBDataControll::bottomChunkLoaded,
    //         m_uiModel, &SensorModel::onBottomChunkLoaded);

    // connect(m_dbController.data(), &DBDataControll::topChunkLoaded,
    //         m_uiModel, &SensorModel::onTopChunkLoaded);
}

void ThreadOrchestrator::startAll() {
    // Запускаем Event Loop во всех трех фоновых потоках
    m_simulatorThread.start();
    m_receiverThread.start();
    m_sqlThread.start();
}

void ThreadOrchestrator::stopAll() {
    // 1. Сначала просим генератор перестать спамить тики данных
    m_simulator->stopGeneration();

    // 2. Сигнализируем всем потокам команду на корректный выход из очередей событий
    if (m_simulatorThread.isRunning()) { m_simulatorThread.quit(); m_simulatorThread.wait(); }
    if (m_receiverThread.isRunning())  { m_receiverThread.quit();  m_receiverThread.wait(); }
    if (m_sqlThread.isRunning())       { m_sqlThread.quit();       m_sqlThread.wait(); }
}

void ThreadOrchestrator::onConnectRequested() {
    // Безопасно запускаем метод генератора в его родном потоке
    QMetaObject::invokeMethod(m_simulator.data(), "startGeneration", Qt::QueuedConnection);
}

void ThreadOrchestrator::onFilterRequested(const QString &filterCondition) {
    // Перенаправляем тяжелый SQL-запрос фильтрации в поток базы данных
    // QMetaObject::invokeMethod(m_dbController.data(), "applyFilterQuery",
    //                           Qt::QueuedConnection, Q_ARG(QString, filterCondition));
}