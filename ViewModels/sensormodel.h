#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include "Data/sensordata.h"
#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>

// Перечисление для отслеживания активного поля сортировки
enum class SortField { Id, Timestamp, Value };

// Предварительное объявление контроллера БД, чтобы не было циклической зависимости
class DBDataControll;

class SensorModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit SensorModel(QObject *parent = nullptr);
    ~SensorModel() = default;

    // Обязательные системные методы для QTableView
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Перехват клика по колонке таблицы для сортировки
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    // Связка с контроллером базы данных
    void setDbController(DBDataControll* dbController);

public slots:
    // Слот приема ЖИВЫХ данных от устройств из другого потока
    void onLiveRecordsReceived(const QVector<SensorData> &liveBatch);

    // Слоты, которые вызываются, когда SQL-поток вернул пачку данных из истории
    void onBottomChunkLoaded(const QVector<SensorData> &chunk);
    void onTopChunkLoaded(const QVector<SensorData> &chunk);

private:
    // Методы сдвига скользящего окна
    void slideWindowDown(int count);
    void slideWindowUp(int count);

    QVector<SensorData> m_records; // Окно памяти в ОЗУ (хранит структуры SensorData)
    DBDataControll* m_dbController; // Указатель на модуль работы с SQL

    // Состояние границ данных
    bool m_reachedTop;
    bool m_reachedBottom;
    bool m_isBufferLoading;

    // Текущий режим сортировки в рантайме
    SortField m_currentSortField;
    Qt::SortOrder m_currentSortOrder;

    // Константы конфигурации окна
    const int m_maxWindowSize;    // Максимальный размер ОЗУ-кэша (например, 500 строк)
    const int m_chunkSize;        // Размер одной подгружаемой пачки (40 строк)
    const int m_triggerThreshold; // Порог предзагрузки до края экрана (10 строк)
};

#endif // SENSORMODEL_H
