#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include "BaggageRecord.h"

/**
 * @brief Класс для работы с PostgreSQL базой данных
 * Управляет подключением и операциями с таблицей baggage_records
 */
class DatabaseManager {
public:
    static DatabaseManager& instance();

    // Инициализация и подключение
    bool connectToDatabase(const QString& host = "localhost",
                          int port = 5432,
                          const QString& dbName = "baggage_db",
                          const QString& user = "postgres",
                          const QString& password = "postgres");

    void disconnectFromDatabase();
    bool isConnected() const;

    // Функция 1: Создать таблицу (инициализация БД)
    bool createTable();

    // Функция 2: Получить все записи
    QVector<BaggageRecord> getAllRecords();

    // Функция 3: Фильтр - пассажиры с 1 вещью 20-30 кг
    QVector<BaggageRecord> filterPassengersWithSingleItem20_30kg();

    // Функция 4: Создать файл сводки (номер рейса, ФИО, общий вес)
    bool createSummaryFile(const QString& filename);

    // Функция 6: Добавить запись
    bool addRecord(const BaggageRecord& record);

    // Функция 7: Удалить записи по номерам рейсов
    int deleteRecordsByFlightNumbers(const QStringList& flightNumbers);

    // Функция 8: Изменить количество вещей для указанных ФИО
    bool changeItemCountByName(const QString& passengerName,
                               const QVector<double>& newWeights);

    // Вспомогательные методы
    void clearAllRecords();
    int getRecordCount();
    QString getLastError() const { return m_lastError; }

    // Поиск
    QVector<BaggageRecord> findRecordsByFlightNumber(const QString& flightNumber);
    QVector<BaggageRecord> findRecordsByPassengerName(const QString& passengerName);

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
    QString m_lastError;

    // Вспомогательные методы
    BaggageRecord recordFromQuery(class QSqlQuery& query);
    QString weightsToString(const QVector<double>& weights);
    QVector<double> weightsFromString(const QString& weightsStr);
};

#endif // DATABASEMANAGER_H
