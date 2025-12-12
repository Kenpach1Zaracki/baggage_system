#ifndef ADDRECORDDIALOG_H
#define ADDRECORDDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVector>
#include <QLabel>
#include "BaggageRecord.h"

/**
 * @brief Диалоговое окно для добавления новой записи о багаже
 */
class AddRecordDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddRecordDialog(QWidget* parent = nullptr);
    ~AddRecordDialog();

    BaggageRecord getRecord() const;

private slots:
    void onItemCountChanged(int count);
    void onAccept();

private:
    void createUI();
    bool validateInput();

    QLineEdit* m_flightNumberEdit;
    QLineEdit* m_passengerNameEdit;
    QSpinBox* m_itemCountSpinBox;
    QVector<QDoubleSpinBox*> m_weightSpinBoxes;
    QVector<QLabel*> m_weightLabels;

    static constexpr int MAX_ITEMS = 5;
};

#endif // ADDRECORDDIALOG_H
