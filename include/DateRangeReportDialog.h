#ifndef DATERANGEREPORTDIALOG_H
#define DATERANGEREPORTDIALOG_H

#include <QDialog>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QDateTime>
#include "BaggageRecord.h"

/**
 * @brief Диалог для создания отчётов за период времени
 * ТЗ п. 1.2.4.1.1 - формирование отчётов за указанный период
 */
class DateRangeReportDialog : public QDialog {
    Q_OBJECT

public:
    explicit DateRangeReportDialog(QWidget *parent = nullptr);
    ~DateRangeReportDialog();

private slots:
    void onGenerateReport();
    void onExportToFile();

private:
    // UI элементы
    QDateTimeEdit* m_dateFromEdit;
    QDateTimeEdit* m_dateToEdit;
    QPushButton* m_generateButton;
    QPushButton* m_exportButton;
    QTextEdit* m_reportTextEdit;
    QLabel* m_statusLabel;

    // Данные отчёта
    QString m_currentReport;

    // Методы
    void setupUI();
    QString generateReportText(const QVector<BaggageRecord>& records,
                              const QDateTime& from,
                              const QDateTime& to);
    void showStatus(const QString& message, bool isError = false);
};

#endif // DATERANGEREPORTDIALOG_H
