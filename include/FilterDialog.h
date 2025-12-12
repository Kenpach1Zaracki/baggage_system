#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include "BaggageRecord.h"

/**
 * @brief Диалоговое окно для отображения отфильтрованных записей
 * (пассажиры с 1 вещью весом 20-30 кг)
 */
class FilterDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilterDialog(const QVector<BaggageRecord>& records, QWidget* parent = nullptr);
    ~FilterDialog();

private:
    void createUI();
    void populateTable();

    QTableWidget* m_tableWidget;
    QVector<BaggageRecord> m_records;
};

#endif // FILTERDIALOG_H
