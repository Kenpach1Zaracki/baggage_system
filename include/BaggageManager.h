#ifndef BAGGAGEMANAGER_H
#define BAGGAGEMANAGER_H

#include "BaggageRecord.h"
#include <QVector>
#include <QString>
#include <memory>

/**
 * @brief Класс для управления коллекцией записей о багаже
 * Реализует все 8 требуемых функций
 */
class BaggageManager {
public:
    BaggageManager();
    ~BaggageManager();

    // Функция 1: Создать файл с заданной структурой записи
    bool createFile(const QString& filename);

    // Функция 2: Загрузить содержимое файла
    bool loadFromFile(const QString& filename);

    // Сохранить данные в файл
    bool saveToFile(const QString& filename);

    // Функция 3: Получить список пассажиров с 1 вещью весом 20-30 кг
    QVector<BaggageRecord> filterPassengersWithSingleItem20_30kg() const;

    // Функция 4: Сформировать файл с номером рейса, ФИО и общим весом багажа
    bool createSummaryFile(const QString& filename);

    // Функция 6: Добавить запись
    bool addRecord(const BaggageRecord& record);

    // Функция 7: Удалить записи по заданным номерам рейсов
    int deleteRecordsByFlightNumbers(const QStringList& flightNumbers);

    // Функция 8: Изменить количество вещей для указанных ФИО
    bool changeItemCountByName(const QString& passengerName, const QVector<double>& newWeights);

    // Вспомогательные методы
    const QVector<BaggageRecord>& getRecords() const { return m_records; }
    void clearRecords();
    int getRecordCount() const { return m_records.size(); }
    bool isEmpty() const { return m_records.isEmpty(); }

    // Поиск записей
    QVector<BaggageRecord> findRecordsByFlightNumber(const QString& flightNumber) const;
    QVector<BaggageRecord> findRecordsByPassengerName(const QString& passengerName) const;

private:
    QVector<BaggageRecord> m_records;
    QString m_currentFilename;

    // Вспомогательные методы для работы с файлами
    bool saveBinaryFile(const QString& filename);
    bool loadBinaryFile(const QString& filename);
};

#endif // BAGGAGEMANAGER_H
