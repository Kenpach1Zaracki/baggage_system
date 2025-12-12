#ifndef BAGGAGERECORD_H
#define BAGGAGERECORD_H

#include <QString>
#include <QVector>
#include <QDataStream>

/**
 * @brief Класс для хранения данных об одной записи багажа пассажира
 */
class BaggageRecord {
public:
    static constexpr int MAX_ITEMS = 5; // Максимальное количество вещей

    BaggageRecord();
    BaggageRecord(const QString& flightNumber, const QString& passengerName,
                  const QVector<double>& itemWeights);

    // Геттеры
    QString getFlightNumber() const { return m_flightNumber; }
    QString getPassengerName() const { return m_passengerName; }
    int getItemCount() const { return m_itemWeights.size(); }
    QVector<double> getItemWeights() const { return m_itemWeights; }
    double getTotalWeight() const;

    // Сеттеры
    void setFlightNumber(const QString& flightNumber) { m_flightNumber = flightNumber; }
    void setPassengerName(const QString& passengerName) { m_passengerName = passengerName; }
    bool setItemWeights(const QVector<double>& weights);

    // Валидация
    static bool isValidItemCount(int count);
    static bool isValidWeight(double weight);
    bool isValid() const;

    // Сериализация для сохранения в файл
    friend QDataStream& operator<<(QDataStream& out, const BaggageRecord& record);
    friend QDataStream& operator>>(QDataStream& in, BaggageRecord& record);

private:
    QString m_flightNumber;      // Номер рейса
    QString m_passengerName;     // ФИО пассажира
    QVector<double> m_itemWeights; // Вес каждой вещи (до 5 штук)
};

#endif // BAGGAGERECORD_H
