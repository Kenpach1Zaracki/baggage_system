#include "BaggageRecord.h"

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

bool BaggageRecord::isValid() const {
    if (m_flightNumber.isEmpty() || m_passengerName.isEmpty()) {
        return false;
    }

    if (!isValidItemCount(m_itemWeights.size())) {
        return false;
    }

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
