#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include <QDateTime>
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

    // Отчёты за период (ТЗ п. 1.2.4.1.1)
    QVector<BaggageRecord> getRecordsByDateRange(const QDateTime& from, const QDateTime& to);

    // Доступ к базе данных (для LoginDialog и других компонентов)
    QSqlDatabase& getDatabase() { return m_db; }

private:
    DatabaseManager();
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_db;
    QString m_lastError;

    // Вспомогательные методы
    QVector<double> getItemWeights(int recordId);
};

#endif // DATABASEMANAGER_H
