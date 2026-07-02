#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <cstdint>

/**
 * Выбрана структура, т.к. быстрее работает на уровне L2 и L3 кэша.
 * Для хранения в контейнере более выгодная, чем tuple или класс по памяти
 *
 * ВНИМАНИЕ! Везде передавать как const и по ссылке
 */
struct SensorData {
    static constexpr uint64_t DEFAULT_RECORD_ID = 0;
    static constexpr uint64_t DEFAULT_SENSOR_ID = 0;
    static constexpr uint64_t DEFAULT_TIMESTAMP = 0;
    static constexpr double DEFAULT_VALUE = 0.0;

    uint64_t recordId = DEFAULT_RECORD_ID; // Primary Key и номер записи для сортировки в UI
    uint64_t sensorId = DEFAULT_SENSOR_ID;
    uint64_t timestamp = DEFAULT_TIMESTAMP;
    double value = DEFAULT_VALUE;
};

#endif // SENSORDATA_H
