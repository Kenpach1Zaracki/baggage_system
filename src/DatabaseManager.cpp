#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QVariant>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager() {
    m_db = QSqlDatabase::addDatabase("QPSQL");
}

DatabaseManager::~DatabaseManager() {
    disconnectFromDatabase();
}

bool DatabaseManager::connectToDatabase(const QString& host, int port,
                                       const QString& dbName,
                                       const QString& user,
                                       const QString& password) {
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);

    if (!m_db.open()) {
        m_lastError = "Ошибка подключения к БД: " + m_db.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    qDebug() << "Успешное подключение к PostgreSQL:" << dbName;
    return true;
}

void DatabaseManager::disconnectFromDatabase() {
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "Отключение от БД";
    }
}

bool DatabaseManager::isConnected() const {
    return m_db.isOpen();
}

// Функция 1: Создать таблицу с заданной структурой
bool DatabaseManager::createTable() {
    QSqlQuery query(m_db);

    // Создаем таблицу багажа пассажиров
    QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS baggage_records (
            id SERIAL PRIMARY KEY,
            flight_number VARCHAR(50) NOT NULL,
            passenger_name VARCHAR(255) NOT NULL,
            item_weights TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";

    if (!query.exec(createTableSQL)) {
        m_lastError = "Ошибка создания таблицы: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    // Создаем индексы для ускорения поиска
    query.exec("CREATE INDEX IF NOT EXISTS idx_flight_number ON baggage_records(flight_number)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_passenger_name ON baggage_records(passenger_name)");

    qDebug() << "Таблица baggage_records создана успешно";
    return true;
}

// Функция 2: Получить все записи
QVector<BaggageRecord> DatabaseManager::getAllRecords() {
    QVector<BaggageRecord> records;
    QSqlQuery query(m_db);

    if (!query.exec("SELECT flight_number, passenger_name, item_weights FROM baggage_records ORDER BY id")) {
        m_lastError = "Ошибка получения записей: " + query.lastError().text();
        qWarning() << m_lastError;
        return records;
    }

    while (query.next()) {
        records.append(recordFromQuery(query));
    }

    return records;
}

// Функция 3: Фильтр пассажиров с 1 вещью весом 20-30 кг
QVector<BaggageRecord> DatabaseManager::filterPassengersWithSingleItem20_30kg() {
    QVector<BaggageRecord> result;
    QVector<BaggageRecord> allRecords = getAllRecords();

    for (const BaggageRecord& record : allRecords) {
        if (record.getItemCount() == 1) {
            double weight = record.getItemWeights()[0];
            if (weight >= 20.0 && weight <= 30.0) {
                result.append(record);
            }
        }
    }

    return result;
}

// Функция 4: Сформировать файл сводки
bool DatabaseManager::createSummaryFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = "Не удалось создать файл сводки: " + filename;
        qWarning() << m_lastError;
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Заголовок
    out << QString("№ рейса\tФ.И.О. пассажира\tОбщий вес багажа (кг)\n");
    out << QString("=======================================================\n");

    // Получаем все записи
    QVector<BaggageRecord> records = getAllRecords();

    // Записываем данные
    for (const BaggageRecord& record : records) {
        out << record.getFlightNumber() << "\t"
            << record.getPassengerName() << "\t"
            << QString::number(record.getTotalWeight(), 'f', 2) << "\n";
    }

    file.close();
    return true;
}

// Функция 6: Добавить запись
bool DatabaseManager::addRecord(const BaggageRecord& record) {
    if (!record.isValid()) {
        m_lastError = "Попытка добавить невалидную запись";
        qWarning() << m_lastError;
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("INSERT INTO baggage_records (flight_number, passenger_name, item_weights) "
                 "VALUES (:flight_number, :passenger_name, :item_weights)");

    query.bindValue(":flight_number", record.getFlightNumber());
    query.bindValue(":passenger_name", record.getPassengerName());
    query.bindValue(":item_weights", weightsToString(record.getItemWeights()));

    if (!query.exec()) {
        m_lastError = "Ошибка добавления записи: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    return true;
}

// Функция 7: Удалить записи по номерам рейсов
int DatabaseManager::deleteRecordsByFlightNumbers(const QStringList& flightNumbers) {
    if (flightNumbers.isEmpty()) {
        return 0;
    }

    QSqlQuery query(m_db);

    // Формируем список placeholders
    QStringList placeholders;
    for (int i = 0; i < flightNumbers.size(); ++i) {
        placeholders.append("?");
    }

    QString sql = QString("DELETE FROM baggage_records WHERE flight_number IN (%1)")
                      .arg(placeholders.join(", "));

    query.prepare(sql);
    for (const QString& flightNumber : flightNumbers) {
        query.addBindValue(flightNumber);
    }

    if (!query.exec()) {
        m_lastError = "Ошибка удаления записей: " + query.lastError().text();
        qWarning() << m_lastError;
        return 0;
    }

    return query.numRowsAffected();
}

// Функция 8: Изменить количество вещей для указанных ФИО
bool DatabaseManager::changeItemCountByName(const QString& passengerName,
                                           const QVector<double>& newWeights) {
    if (!BaggageRecord::isValidItemCount(newWeights.size())) {
        m_lastError = "Неверное количество вещей";
        return false;
    }

    for (double weight : newWeights) {
        if (!BaggageRecord::isValidWeight(weight)) {
            m_lastError = "Неверный вес вещи";
            return false;
        }
    }

    QSqlQuery query(m_db);
    query.prepare("UPDATE baggage_records SET item_weights = :item_weights "
                 "WHERE passenger_name = :passenger_name");

    query.bindValue(":item_weights", weightsToString(newWeights));
    query.bindValue(":passenger_name", passengerName);

    if (!query.exec()) {
        m_lastError = "Ошибка изменения записей: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    int affected = query.numRowsAffected();
    if (affected == 0) {
        m_lastError = "Пассажир с указанным ФИО не найден";
        return false;
    }

    qDebug() << "Изменено записей:" << affected;
    return true;
}

void DatabaseManager::clearAllRecords() {
    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM baggage_records")) {
        m_lastError = "Ошибка очистки таблицы: " + query.lastError().text();
        qWarning() << m_lastError;
    }
}

int DatabaseManager::getRecordCount() {
    QSqlQuery query(m_db);
    if (!query.exec("SELECT COUNT(*) FROM baggage_records")) {
        m_lastError = "Ошибка подсчета записей: " + query.lastError().text();
        qWarning() << m_lastError;
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

QVector<BaggageRecord> DatabaseManager::findRecordsByFlightNumber(const QString& flightNumber) {
    QVector<BaggageRecord> records;
    QSqlQuery query(m_db);

    query.prepare("SELECT flight_number, passenger_name, item_weights "
                 "FROM baggage_records WHERE flight_number = :flight_number");
    query.bindValue(":flight_number", flightNumber);

    if (!query.exec()) {
        m_lastError = "Ошибка поиска по номеру рейса: " + query.lastError().text();
        qWarning() << m_lastError;
        return records;
    }

    while (query.next()) {
        records.append(recordFromQuery(query));
    }

    return records;
}

QVector<BaggageRecord> DatabaseManager::findRecordsByPassengerName(const QString& passengerName) {
    QVector<BaggageRecord> records;
    QSqlQuery query(m_db);

    query.prepare("SELECT flight_number, passenger_name, item_weights "
                 "FROM baggage_records WHERE passenger_name = :passenger_name");
    query.bindValue(":passenger_name", passengerName);

    if (!query.exec()) {
        m_lastError = "Ошибка поиска по ФИО: " + query.lastError().text();
        qWarning() << m_lastError;
        return records;
    }

    while (query.next()) {
        records.append(recordFromQuery(query));
    }

    return records;
}

// Вспомогательные методы
BaggageRecord DatabaseManager::recordFromQuery(QSqlQuery& query) {
    QString flightNumber = query.value("flight_number").toString();
    QString passengerName = query.value("passenger_name").toString();
    QString weightsStr = query.value("item_weights").toString();
    QVector<double> weights = weightsFromString(weightsStr);

    return BaggageRecord(flightNumber, passengerName, weights);
}

QString DatabaseManager::weightsToString(const QVector<double>& weights) {
    QStringList list;
    for (double weight : weights) {
        list.append(QString::number(weight, 'f', 2));
    }
    return list.join(",");
}

QVector<double> DatabaseManager::weightsFromString(const QString& weightsStr) {
    QVector<double> weights;
    QStringList list = weightsStr.split(",", Qt::SkipEmptyParts);

    for (const QString& str : list) {
        bool ok;
        double weight = str.toDouble(&ok);
        if (ok) {
            weights.append(weight);
        }
    }

    return weights;
}
