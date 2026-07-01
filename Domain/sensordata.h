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
    uint64_t recordId = 0; // Primary Key и номер записи для сортировки в UI
    uint64_t sensorId = 0;
    uint64_t timestamp = 0;
    double   value = 0.0;
};

#endif // SENSORDATA_H
