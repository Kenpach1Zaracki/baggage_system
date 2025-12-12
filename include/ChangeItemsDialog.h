#ifndef CHANGEITEMSDIALOG_H
#define CHANGEITEMSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QVector>
#include <QLabel>

/**
 * @brief Диалоговое окно для изменения количества вещей по ФИО пассажира
 */
class ChangeItemsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChangeItemsDialog(QWidget* parent = nullptr);
    ~ChangeItemsDialog();

    QString getPassengerName() const;
    QVector<double> getItemWeights() const;

private slots:
    void onItemCountChanged(int count);
    void onAccept();

private:
    void createUI();
    bool validateInput();

    QLineEdit* m_passengerNameEdit;
    QSpinBox* m_itemCountSpinBox;
    QVector<QDoubleSpinBox*> m_weightSpinBoxes;
    QVector<QLabel*> m_weightLabels;

    static constexpr int MAX_ITEMS = 5;
};

#endif // CHANGEITEMSDIALOG_H
