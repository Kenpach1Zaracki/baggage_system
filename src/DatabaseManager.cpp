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
    QString createRecordsSQL = R"(
        CREATE TABLE IF NOT EXISTS baggage_records (
            id SERIAL PRIMARY KEY,
            flight_number VARCHAR(50) NOT NULL,
            passenger_name VARCHAR(255) NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            created_by INTEGER
        )
    )";

    if (!query.exec(createRecordsSQL)) {
        m_lastError = "Ошибка создания таблицы baggage_records: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    // Создаем таблицу вещей (нормализация)
    QString createItemsSQL = R"(
        CREATE TABLE IF NOT EXISTS baggage_items (
            id SERIAL PRIMARY KEY,
            baggage_record_id INTEGER NOT NULL REFERENCES baggage_records(id) ON DELETE CASCADE,
            item_number INTEGER NOT NULL CHECK (item_number >= 1 AND item_number <= 5),
            weight NUMERIC(5,2) NOT NULL CHECK (weight > 0 AND weight <= 100),
            UNIQUE(baggage_record_id, item_number)
        )
    )";

    if (!query.exec(createItemsSQL)) {
        m_lastError = "Ошибка создания таблицы baggage_items: " + query.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    // Создаем индексы для ускорения поиска
    query.exec("CREATE INDEX IF NOT EXISTS idx_flight_number ON baggage_records(flight_number)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_passenger_name ON baggage_records(passenger_name)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_baggage_items_record ON baggage_items(baggage_record_id)");

    qDebug() << "Таблицы baggage_records и baggage_items созданы успешно";
    return true;
}

// Функция 2: Получить все записи
QVector<BaggageRecord> DatabaseManager::getAllRecords() {
    QVector<BaggageRecord> records;
    QSqlQuery query(m_db);

    // Получаем все записи багажа
    if (!query.exec("SELECT id, flight_number, passenger_name FROM baggage_records ORDER BY id")) {
        m_lastError = "Ошибка получения записей: " + query.lastError().text();
        qWarning() << m_lastError;
        return records;
    }

    while (query.next()) {
        int recordId = query.value("id").toInt();
        QString flightNumber = query.value("flight_number").toString();
        QString passengerName = query.value("passenger_name").toString();

        // Получаем веса для этой записи
        QVector<double> weights = getItemWeights(recordId);

        records.append(BaggageRecord(flightNumber, passengerName, weights));
    }

    return records;
}

// Вспомогательный метод для получения весов вещей
QVector<double> DatabaseManager::getItemWeights(int recordId) {
    QVector<double> weights;
    QSqlQuery query(m_db);

    query.prepare("SELECT weight FROM baggage_items WHERE baggage_record_id = ? ORDER BY item_number");
    query.addBindValue(recordId);

    if (!query.exec()) {
        qWarning() << "Ошибка получения весов для записи" << recordId << ":" << query.lastError().text();
        return weights;
    }

    while (query.next()) {
        weights.append(query.value(0).toDouble());
    }

    return weights;
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

        // Проверка ошибок записи
        if (out.status() != QTextStream::Ok) {
            m_lastError = "Ошибка записи в файл сводки: " + filename;
            qWarning() << m_lastError;
            file.close();
            return false;
        }
    }

    file.close();

    // Проверка успешности закрытия файла
    if (file.error() != QFileDevice::NoError) {
        m_lastError = "Ошибка при закрытии файла: " + file.errorString();
        qWarning() << m_lastError;
        return false;
    }

    qDebug() << "Файл сводки успешно создан:" << filename;
    return true;
}

// Функция 6: Добавить запись
bool DatabaseManager::addRecord(const BaggageRecord& record) {
    if (!record.isValid()) {
        m_lastError = "Попытка добавить невалидную запись";
        qWarning() << m_lastError;
        return false;
    }

    // Проверка подключения к БД
    if (!m_db.isOpen()) {
        m_lastError = "База данных не подключена";
        qWarning() << m_lastError;
        return false;
    }

    // ТРАНЗАКЦИЯ: начинаем транзакцию
    if (!m_db.transaction()) {
        m_lastError = "Не удалось начать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    // Вставляем запись багажа
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO baggage_records (flight_number, passenger_name) VALUES (?, ?) RETURNING id");
    query.addBindValue(record.getFlightNumber());
    query.addBindValue(record.getPassengerName());

    if (!query.exec()) {
        m_lastError = "Ошибка добавления записи: " + query.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();
        return false;
    }

    // Получаем ID созданной записи
    int recordId = -1;
    if (query.next()) {
        recordId = query.value(0).toInt();
    } else {
        m_lastError = "Не удалось получить ID созданной записи";
        qWarning() << m_lastError;
        m_db.rollback();
        return false;
    }

    // Вставляем вещи
    QVector<double> weights = record.getItemWeights();
    for (int i = 0; i < weights.size(); ++i) {
        QSqlQuery itemQuery(m_db);
        itemQuery.prepare("INSERT INTO baggage_items (baggage_record_id, item_number, weight) VALUES (?, ?, ?)");
        itemQuery.addBindValue(recordId);
        itemQuery.addBindValue(i + 1);
        itemQuery.addBindValue(weights[i]);

        if (!itemQuery.exec()) {
            m_lastError = "Ошибка добавления вещи: " + itemQuery.lastError().text();
            qWarning() << m_lastError;
            m_db.rollback();
            return false;
        }
    }

    // Фиксируем транзакцию
    if (!m_db.commit()) {
        m_lastError = "Не удалось зафиксировать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();
        return false;
    }

    qDebug() << "Запись и вещи успешно добавлены:" << record.getPassengerName();
    return true;
}

// Функция 7: Удалить записи по номерам рейсов
int DatabaseManager::deleteRecordsByFlightNumbers(const QStringList& flightNumbers) {
    if (flightNumbers.isEmpty()) {
        return 0;
    }

    // Проверка подключения к БД
    if (!m_db.isOpen()) {
        m_lastError = "База данных не подключена";
        qWarning() << m_lastError;
        return 0;
    }

    // ТРАНЗАКЦИЯ: Начинаем транзакцию для атомарности операции
    if (!m_db.transaction()) {
        m_lastError = "Не удалось начать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
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
        m_db.rollback();  // Откатываем изменения
        return 0;
    }

    int affectedRows = query.numRowsAffected();

    // Фиксируем транзакцию
    if (!m_db.commit()) {
        m_lastError = "Не удалось зафиксировать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();
        return 0;
    }

    qDebug() << "Транзакция успешно выполнена. Удалено записей:" << affectedRows;
    return affectedRows;
}

// Функция 8: Изменить количество вещей для указанных ФИО
bool DatabaseManager::changeItemCountByName(const QString& passengerName,
                                           const QVector<double>& newWeights) {
    // Валидация входных данных
    if (passengerName.trimmed().isEmpty()) {
        m_lastError = "ФИО пассажира не может быть пустым";
        return false;
    }

    if (!BaggageRecord::isValidItemCount(newWeights.size())) {
        m_lastError = "Неверное количество вещей (должно быть от 1 до 5)";
        return false;
    }

    for (double weight : newWeights) {
        if (!BaggageRecord::isValidWeight(weight)) {
            m_lastError = "Неверный вес вещи (должен быть от 0 до 100 кг)";
            return false;
        }
    }

    // Проверка подключения к БД
    if (!m_db.isOpen()) {
        m_lastError = "База данных не подключена";
        qWarning() << m_lastError;
        return false;
    }

    // ТРАНЗАКЦИЯ: Начинаем транзакцию
    if (!m_db.transaction()) {
        m_lastError = "Не удалось начать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
        return false;
    }

    // Получаем ID записи по ФИО
    QSqlQuery findQuery(m_db);
    findQuery.prepare("SELECT id FROM baggage_records WHERE passenger_name = ?");
    findQuery.addBindValue(passengerName);

    if (!findQuery.exec()) {
        m_lastError = "Ошибка поиска записи: " + findQuery.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();
        return false;
    }

    if (!findQuery.next()) {
        m_lastError = "Пассажир с указанным ФИО не найден";
        m_db.rollback();
        return false;
    }

    int recordId = findQuery.value(0).toInt();

    // Удаляем старые вещи
    QSqlQuery deleteQuery(m_db);
    deleteQuery.prepare("DELETE FROM baggage_items WHERE baggage_record_id = ?");
    deleteQuery.addBindValue(recordId);

    if (!deleteQuery.exec()) {
        m_lastError = "Ошибка удаления старых вещей: " + deleteQuery.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();
        return false;
    }

    // Вставляем новые вещи
    for (int i = 0; i < newWeights.size(); ++i) {
        QSqlQuery insertQuery(m_db);
        insertQuery.prepare("INSERT INTO baggage_items (baggage_record_id, item_number, weight) VALUES (?, ?, ?)");
        insertQuery.addBindValue(recordId);
        insertQuery.addBindValue(i + 1);
        insertQuery.addBindValue(newWeights[i]);

        if (!insertQuery.exec()) {
            m_lastError = "Ошибка вставки новой вещи: " + insertQuery.lastError().text();
            qWarning() << m_lastError;
            m_db.rollback();
            return false;
        }
    }

    // Обновляем updated_at
    QSqlQuery updateQuery(m_db);
    updateQuery.prepare("UPDATE baggage_records SET updated_at = CURRENT_TIMESTAMP WHERE id = ?");
    updateQuery.addBindValue(recordId);
    updateQuery.exec();

    // Фиксируем транзакцию
    if (!m_db.commit()) {
        m_lastError = "Не удалось зафиксировать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();
        return false;
    }

    qDebug() << "Транзакция успешно выполнена. Обновлены веса для:" << passengerName;
    return true;
}

void DatabaseManager::clearAllRecords() {
    // Проверка подключения к БД
    if (!m_db.isOpen()) {
        m_lastError = "База данных не подключена";
        qWarning() << m_lastError;
        return;
    }

    // ТРАНЗАКЦИЯ: Начинаем транзакцию для безопасного удаления всех записей
    if (!m_db.transaction()) {
        m_lastError = "Не удалось начать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
        return;
    }

    QSqlQuery query(m_db);
    if (!query.exec("DELETE FROM baggage_records")) {
        m_lastError = "Ошибка очистки таблицы: " + query.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();  // Откатываем изменения
        return;
    }

    // Фиксируем транзакцию
    if (!m_db.commit()) {
        m_lastError = "Не удалось зафиксировать транзакцию: " + m_db.lastError().text();
        qWarning() << m_lastError;
        m_db.rollback();
        return;
    }

    qDebug() << "Транзакция успешно выполнена. Все записи удалены.";
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

    query.prepare("SELECT id, flight_number, passenger_name FROM baggage_records WHERE flight_number = ?");
    query.addBindValue(flightNumber);

    if (!query.exec()) {
        m_lastError = "Ошибка поиска по номеру рейса: " + query.lastError().text();
        qWarning() << m_lastError;
        return records;
    }

    while (query.next()) {
        int recordId = query.value("id").toInt();
        QString flightNum = query.value("flight_number").toString();
        QString passengerName = query.value("passenger_name").toString();
        QVector<double> weights = getItemWeights(recordId);

        records.append(BaggageRecord(flightNum, passengerName, weights));
    }

    return records;
}

QVector<BaggageRecord> DatabaseManager::findRecordsByPassengerName(const QString& passengerName) {
    QVector<BaggageRecord> records;
    QSqlQuery query(m_db);

    query.prepare("SELECT id, flight_number, passenger_name FROM baggage_records WHERE passenger_name = ?");
    query.addBindValue(passengerName);

    if (!query.exec()) {
        m_lastError = "Ошибка поиска по ФИО: " + query.lastError().text();
        qWarning() << m_lastError;
        return records;
    }

    while (query.next()) {
        int recordId = query.value("id").toInt();
        QString flightNumber = query.value("flight_number").toString();
        QString passName = query.value("passenger_name").toString();
        QVector<double> weights = getItemWeights(recordId);

        records.append(BaggageRecord(flightNumber, passName, weights));
    }

    return records;
}

QVector<BaggageRecord> DatabaseManager::getRecordsByDateRange(const QDateTime& from, const QDateTime& to) {
    QVector<BaggageRecord> records;
    QSqlQuery query(m_db);

    query.prepare("SELECT id, flight_number, passenger_name "
                 "FROM baggage_records "
                 "WHERE created_at BETWEEN ? AND ? "
                 "ORDER BY created_at");
    query.addBindValue(from);
    query.addBindValue(to);

    if (!query.exec()) {
        m_lastError = "Ошибка получения записей за период: " + query.lastError().text();
        qWarning() << m_lastError;
        return records;
    }

    while (query.next()) {
        int recordId = query.value("id").toInt();
        QString flightNumber = query.value("flight_number").toString();
        QString passengerName = query.value("passenger_name").toString();
        QVector<double> weights = getItemWeights(recordId);

        records.append(BaggageRecord(flightNumber, passengerName, weights));
    }

    return records;
}
