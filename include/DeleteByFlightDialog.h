#ifndef DELETEBYFLIGHTDIALOG_H
#define DELETEBYFLIGHTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>

/**
 * @brief Диалоговое окно для удаления записей по номерам рейсов
 */
class DeleteByFlightDialog : public QDialog {
    Q_OBJECT

public:
    explicit DeleteByFlightDialog(QWidget* parent = nullptr);
    ~DeleteByFlightDialog();

    QStringList getFlightNumbers() const;

private slots:
    void onAccept();

private:
    void createUI();
    bool validateInput();

    QTextEdit* m_flightNumbersEdit;
};

#endif // DELETEBYFLIGHTDIALOG_H
