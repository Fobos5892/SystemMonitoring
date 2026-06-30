#include "sensormodel.h"
#include "DBModel/dbdatacontroll.h"
#include <algorithm>

SensorModel::SensorModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_dbController(nullptr)
    , m_reachedTop(false)
    , m_reachedBottom(false)
    , m_isBufferLoading(false)
    , m_currentSortField(SortField::Id)
    , m_currentSortOrder(Qt::AscendingOrder)
    , m_maxWindowSize(500)
    , m_chunkSize(40)
    , m_triggerThreshold(10)
{
}

void SensorModel::setDbController(DBDataControll* dbController) {
    m_dbController = dbController;
}

int SensorModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_records.size();
}

int SensorModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 3; // Три колонки: ID датчика, Значение, Время
}

// Главный метод отрисовки ячеек + триггер опережающего кэширования (Prefetch)
QVariant SensorModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || role != Qt::DisplayRole) return QVariant();

    int currentRow = index.row();
    int bufferSize = m_records.size();

    // УМНАЯ ПРЕДЗАГРУЗКА: Проверяем, близко ли пользователь к краю кэша ОЗУ
    if (!m_isBufferLoading && m_dbController) {
        // Подползаем к нижнему краю — подгружаем данные в конец, удаляем из начала
        if (currentRow >= bufferSize - m_triggerThreshold) {
            const_cast<SensorModel*>(this)->slideWindowDown(m_chunkSize);
        }
        // Подползаем к верхнему краю — подгружаем данные в начало, удаляем из конца
        else if (currentRow <= m_triggerThreshold && bufferSize >= m_maxWindowSize) {
            const_cast<SensorModel*>(this)->slideWindowUp(m_chunkSize);
        }
    }

    // Возврат данных для конкретной ячейки
    const auto &record = m_records[currentRow];
    switch (index.column()) {
    case 0: return record.id; // uint64_t ID устройства
    case 1: return record.value; // double Значение
    case 2: return QLocale::system().toString(QDateTime::fromMSecsSinceEpoch(record.timestamp), "yyyy-MM-dd HH:mm:ss");

    }
    return QVariant();
}

QVariant SensorModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) return QVariant();
    switch (section) {
    case 0: return "ID Датчика";
    case 1: return "Значение";
    case 2: return "Дата и Время";
    }
    return QVariant();
}

// Метод сортировки: вызывается автоматически при клике по шапке QTableView
void SensorModel::sort(int column, Qt::SortOrder order) {
    beginResetModel();

    m_currentSortOrder = order;
    if (column == 0) m_currentSortField = SortField::Id;
    else if (column == 1) m_currentSortField = SortField::Value;
    else if (column == 2) m_currentSortField = SortField::Timestamp;

    // Быстрая сортировка текущего видимого окна в ОЗУ с помощью лямбды
    auto comparator = [this](const SensorData &a, const SensorData &b) {
        bool isLessThan = false;
        if (m_currentSortField == SortField::Id) isLessThan = a.id < b.id;
        else if (m_currentSortField == SortField::Value) isLessThan = a.value < b.value;
        else if (m_currentSortField == SortField::Timestamp) isLessThan = a.timestamp < b.timestamp;

        return m_currentSortOrder == Qt::AscendingOrder ? isLessThan : !isLessThan;
    };

    std::sort(m_records.begin(), m_records.end(), comparator);

    // Сбрасываем флаги тупиков, так как срез данных изменился
    m_reachedTop = false;
    m_reachedBottom = false;

    endResetModel();

    // ВАЖНО: Тут отправляется команда в DBDataControll, чтобы он перестроил будущие
    // SQL запросы под ORDER BY в зависимости от выбранного m_currentSortField
}

// Прием данных реального времени из потока устройств
void SensorModel::onLiveRecordsReceived(const QVector<SensorData> &liveBatch) {
    if (liveBatch.isEmpty()) return;
    m_reachedBottom = false;

    // Живой онлайн-поток "в самый конец экрана" пускаем только при дефолтной сортировке.
    // Если пользователь включил кастомную сортировку по значению, онлайн-поток замораживается на экране
    // (данные пишутся только в SQL), чтобы строки хаотично не прыгали перед глазами.
    bool isDefaultSorting = (m_currentSortField == SortField::Id || m_currentSortField == SortField::Timestamp)
                            && m_currentSortOrder == Qt::DescendingOrder;

    if (!isDefaultSorting) return;

    int inputSize = liveBatch.size();

    // Контроль ОЗУ: выталкиваем старые строки СВЕРХУ, освобождая место под новые
    if (m_records.size() + inputSize > m_maxWindowSize) {
        int rowsToRemove = (m_records.size() + inputSize) - m_maxWindowSize;
        beginRemoveRows(QModelIndex(), 0, rowsToRemove - 1);
        m_records.remove(0, rowsToRemove);
        endRemoveRows();
    }

    // Вставляем свежую пачку от устройств в самый конец таблицы
    int insertPos = m_records.size();
    beginInsertRows(QModelIndex(), insertPos, insertPos + inputSize - 1);
    m_records.append(liveBatch);
    endInsertRows();
}

void SensorModel::slideWindowDown(int count) {
    if (m_records.isEmpty() || m_reachedBottom) return;
    m_isBufferLoading = true;

    // Запрашиваем данные у контроллера БД асинхронно
    // (В реальной многопоточности этот вызов делается через QMetaObject::invokeMethod в поток SQL)
    // QMetaObject::invokeMethod(m_dbController, "fetchRangeBelow",
    //                           Q_ARG(uint64_t, m_records.last().id), Q_ARG(int, count));
    Q_UNUSED(count);
}

void SensorModel::slideWindowUp(int count) {
    if (m_records.isEmpty() || m_reachedTop) return;
    m_isBufferLoading = true;

    // QMetaObject::invokeMethod(m_dbController, "fetchRangeAbove",
    //                           Q_ARG(uint64_t, m_records.first().id), Q_ARG(int, count));
    Q_UNUSED(count);
}

// Слот вызывается, когда поток SQL успешно выгрузил чанк данных СНИЗУ
void SensorModel::onBottomChunkLoaded(const QVector<SensorData> &chunk) {
    if (chunk.isEmpty()) {
        m_reachedBottom = true;
        m_isBufferLoading = false;
        return;
    }

    if (chunk.size() < m_chunkSize) m_reachedBottom = true;

    // Сдвигаем окно: удаляем старое сверху, добавляем новое снизу
    beginRemoveRows(QModelIndex(), 0, chunk.size() - 1);
    m_records.remove(0, chunk.size());
    endRemoveRows();
    m_reachedTop = false;

    int insertPos = m_records.size();
    beginInsertRows(QModelIndex(), insertPos, insertPos + chunk.size() - 1);
    m_records.append(chunk);
    endInsertRows();

    m_isBufferLoading = false;
}

// Слот вызывается, когда поток SQL успешно выгрузил чанк данных СВЕРХУ
void SensorModel::onTopChunkLoaded(const QVector<SensorData> &chunk) {
    if (chunk.isEmpty()) {
        m_reachedTop = true;
        m_isBufferLoading = false;
        return;
    }

    if (chunk.size() < m_chunkSize) m_reachedTop = true;

    // Сдвигаем окно: удаляем старое снизу, добавляем новое сверху
    int lastIndex = m_records.size() - 1;
    beginRemoveRows(QModelIndex(), lastIndex - chunk.size() + 1, lastIndex);
    m_records.resize(m_records.size() - chunk.size());
    endRemoveRows();
    m_reachedBottom = false;

    beginInsertRows(QModelIndex(), 0, chunk.size() - 1);
    for (int i = chunk.size() - 1; i >= 0; --i) {
        m_records.prepend(chunk[i]);
    }
    endInsertRows();

    m_isBufferLoading = false;
}