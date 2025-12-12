#include "BaggageManager.h"
#include "DatabaseManager.h"
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDebug>

BaggageManager::BaggageManager() {
}

BaggageManager::~BaggageManager() {
}

// Функция 1: Создать/инициализировать БД
bool BaggageManager::createFile(const QString& filename) {
    Q_UNUSED(filename);
    m_currentFilename = "PostgreSQL Database";

    // Очищаем кеш
    m_records.clear();

    // Очищаем таблицу в БД
    DatabaseManager::instance().clearAllRecords();

    return true;
}

// Функция 2: Загрузить содержимое из БД
bool BaggageManager::loadFromFile(const QString& filename) {
    Q_UNUSED(filename);
    m_records = DatabaseManager::instance().getAllRecords();
    m_currentFilename = "PostgreSQL Database";
    return true;
}

// Сохранить данные (для PostgreSQL это не требуется - данные сохраняются автоматически)
bool BaggageManager::saveToFile(const QString& filename) {
    Q_UNUSED(filename);
    // PostgreSQL сохраняет данные автоматически при каждой операции
    return true;
}

// Функция 3: Получить список пассажиров с 1 вещью весом 20-30 кг
QVector<BaggageRecord> BaggageManager::filterPassengersWithSingleItem20_30kg() const {
    return DatabaseManager::instance().filterPassengersWithSingleItem20_30kg();
}

// Функция 4: Сформировать файл с номером рейса, ФИО и общим весом багажа
bool BaggageManager::createSummaryFile(const QString& filename) {
    return DatabaseManager::instance().createSummaryFile(filename);
}

// Функция 6: Добавить запись
bool BaggageManager::addRecord(const BaggageRecord& record) {
    if (!record.isValid()) {
        qWarning() << "Попытка добавить невалидную запись";
        return false;
    }

    bool success = DatabaseManager::instance().addRecord(record);
    if (success) {
        // Обновляем кеш
        m_records = DatabaseManager::instance().getAllRecords();
    }
    return success;
}

// Функция 7: Удалить записи по заданным номерам рейсов
int BaggageManager::deleteRecordsByFlightNumbers(const QStringList& flightNumbers) {
    int deletedCount = DatabaseManager::instance().deleteRecordsByFlightNumbers(flightNumbers);
    if (deletedCount > 0) {
        // Обновляем кеш
        m_records = DatabaseManager::instance().getAllRecords();
    }
    return deletedCount;
}

// Функция 8: Изменить количество вещей для указанных ФИО
bool BaggageManager::changeItemCountByName(const QString& passengerName,
                                           const QVector<double>& newWeights) {
    bool success = DatabaseManager::instance().changeItemCountByName(passengerName, newWeights);
    if (success) {
        // Обновляем кеш
        m_records = DatabaseManager::instance().getAllRecords();
    }
    return success;
}

// Очистить все записи
void BaggageManager::clearRecords() {
    DatabaseManager::instance().clearAllRecords();
    m_records.clear();
}

// Найти записи по номеру рейса
QVector<BaggageRecord> BaggageManager::findRecordsByFlightNumber(const QString& flightNumber) const {
    return DatabaseManager::instance().findRecordsByFlightNumber(flightNumber);
}

// Найти записи по ФИО пассажира
QVector<BaggageRecord> BaggageManager::findRecordsByPassengerName(const QString& passengerName) const {
    return DatabaseManager::instance().findRecordsByPassengerName(passengerName);
}

// Устаревшие методы для совместимости (больше не используются)
bool BaggageManager::saveBinaryFile(const QString& filename) {
    Q_UNUSED(filename);
    // Данные сохраняются в PostgreSQL автоматически
    return true;
}

bool BaggageManager::loadBinaryFile(const QString& filename) {
    Q_UNUSED(filename);
    // Данные загружаются из PostgreSQL
    return loadFromFile(filename);
}
