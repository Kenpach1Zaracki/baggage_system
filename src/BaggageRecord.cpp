#include "BaggageRecord.h"
#include <QRegularExpression>

BaggageRecord::BaggageRecord()
    : m_flightNumber(""), m_passengerName("") {
}

BaggageRecord::BaggageRecord(const QString& flightNumber, const QString& passengerName,
                             const QVector<double>& itemWeights)
    : m_flightNumber(flightNumber), m_passengerName(passengerName) {
    setItemWeights(itemWeights);
}

double BaggageRecord::getTotalWeight() const {
    double total = 0.0;
    for (double weight : m_itemWeights) {
        total += weight;
    }
    return total;
}

bool BaggageRecord::setItemWeights(const QVector<double>& weights) {
    // Проверка количества вещей
    if (!isValidItemCount(weights.size())) {
        return false;
    }

    // Проверка веса каждой вещи
    for (double weight : weights) {
        if (!isValidWeight(weight)) {
            return false;
        }
    }

    m_itemWeights = weights;
    return true;
}

bool BaggageRecord::isValidItemCount(int count) {
    return count > 0 && count <= MAX_ITEMS;
}

bool BaggageRecord::isValidWeight(double weight) {
    return weight > 0.0 && weight <= 100.0; // Максимальный вес одной вещи 100 кг
}

bool BaggageRecord::isValidFlightNumber(const QString& flightNumber) {
    QString trimmed = flightNumber.trimmed();

    // Проверка: не пустое, длина 4-10 символов
    if (trimmed.isEmpty() || trimmed.length() < 4 || trimmed.length() > 10) {
        return false;
    }

    // Проверка: не содержит опасные символы
    if (trimmed.contains(QRegularExpression("[<>{}\\[\\]$;'\"`\\\\]"))) {
        return false;
    }

    // Проверка формата: 2 буквы + цифры (например SU1234, BA456)
    QRegularExpression regex("^[A-Z]{2}\\d{3,4}$");
    return regex.match(trimmed).hasMatch();
}

bool BaggageRecord::isValidPassengerName(const QString& name) {
    QString trimmed = name.trimmed();

    // Проверка: длина от 3 до 255 символов
    if (trimmed.length() < 3 || trimmed.length() > 255) {
        return false;
    }

    // Проверка: не содержит опасные символы
    if (trimmed.contains(QRegularExpression("[<>{}\\[\\]$;'\"`\\\\]"))) {
        return false;
    }

    // Проверка: содержит хотя бы одну букву
    if (!trimmed.contains(QRegularExpression("[А-Яа-яA-Za-z]"))) {
        return false;
    }

    return true;
}

bool BaggageRecord::isValid() const {
    // Валидация номера рейса
    if (!isValidFlightNumber(m_flightNumber)) {
        return false;
    }

    // Валидация ФИО пассажира
    if (!isValidPassengerName(m_passengerName)) {
        return false;
    }

    // Валидация количества вещей
    if (!isValidItemCount(m_itemWeights.size())) {
        return false;
    }

    // Валидация веса каждой вещи
    for (double weight : m_itemWeights) {
        if (!isValidWeight(weight)) {
            return false;
        }
    }

    return true;
}

// Сериализация для сохранения в бинарный файл
QDataStream& operator<<(QDataStream& out, const BaggageRecord& record) {
    out << record.m_flightNumber;
    out << record.m_passengerName;
    out << record.m_itemWeights;
    return out;
}

QDataStream& operator>>(QDataStream& in, BaggageRecord& record) {
    in >> record.m_flightNumber;
    in >> record.m_passengerName;
    in >> record.m_itemWeights;
    return in;
}
