#include "DateRangeReportDialog.h"
#include "DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QSet>

DateRangeReportDialog::DateRangeReportDialog(QWidget *parent)
    : QDialog(parent) {
    setupUI();
    setWindowTitle("Отчёт за период времени");
    setModal(true);
    setMinimumSize(700, 500);
}

DateRangeReportDialog::~DateRangeReportDialog() {
}

void DateRangeReportDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Заголовок
    QLabel* titleLabel = new QLabel("Формирование отчёта за период", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(10);

    // Группа выбора дат
    QGroupBox* dateGroup = new QGroupBox("Период времени", this);
    QFormLayout* dateLayout = new QFormLayout(dateGroup);

    m_dateFromEdit = new QDateTimeEdit(this);
    m_dateFromEdit->setCalendarPopup(true);
    m_dateFromEdit->setDateTime(QDateTime::currentDateTime().addDays(-30));
    m_dateFromEdit->setDisplayFormat("dd.MM.yyyy HH:mm");
    dateLayout->addRow("С:", m_dateFromEdit);

    m_dateToEdit = new QDateTimeEdit(this);
    m_dateToEdit->setCalendarPopup(true);
    m_dateToEdit->setDateTime(QDateTime::currentDateTime());
    m_dateToEdit->setDisplayFormat("dd.MM.yyyy HH:mm");
    dateLayout->addRow("По:", m_dateToEdit);

    mainLayout->addWidget(dateGroup);

    // Кнопки управления
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_generateButton = new QPushButton("Сформировать отчёт", this);
    m_generateButton->setDefault(true);
    buttonLayout->addWidget(m_generateButton);

    m_exportButton = new QPushButton("Экспорт в файл", this);
    m_exportButton->setEnabled(false);
    buttonLayout->addWidget(m_exportButton);

    mainLayout->addLayout(buttonLayout);

    // Статусная метка
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    // Область для отображения отчёта
    QLabel* reportLabel = new QLabel("Результат отчёта:", this);
    mainLayout->addWidget(reportLabel);

    m_reportTextEdit = new QTextEdit(this);
    m_reportTextEdit->setReadOnly(true);
    m_reportTextEdit->setFont(QFont("Courier", 10));
    mainLayout->addWidget(m_reportTextEdit);

    // Подключение сигналов
    connect(m_generateButton, &QPushButton::clicked, this, &DateRangeReportDialog::onGenerateReport);
    connect(m_exportButton, &QPushButton::clicked, this, &DateRangeReportDialog::onExportToFile);
}

void DateRangeReportDialog::onGenerateReport() {
    QDateTime fromDate = m_dateFromEdit->dateTime();
    QDateTime toDate = m_dateToEdit->dateTime();

    // Валидация дат
    if (fromDate > toDate) {
        showStatus("Ошибка: дата начала больше даты окончания!", true);
        return;
    }

    // Получение записей за период из БД
    DatabaseManager& dbManager = DatabaseManager::instance();
    QVector<BaggageRecord> records = dbManager.getRecordsByDateRange(fromDate, toDate);

    if (records.isEmpty()) {
        showStatus("За указанный период записей не найдено", true);
        m_reportTextEdit->setPlainText("За период с " +
            fromDate.toString("dd.MM.yyyy HH:mm") + " по " +
            toDate.toString("dd.MM.yyyy HH:mm") +
            " записей не найдено.");
        m_exportButton->setEnabled(false);
        return;
    }

    // Генерация текста отчёта
    m_currentReport = generateReportText(records, fromDate, toDate);
    m_reportTextEdit->setPlainText(m_currentReport);

    showStatus(QString("Отчёт сформирован: найдено записей - %1").arg(records.size()), false);
    m_exportButton->setEnabled(true);

    qDebug() << "Отчёт сформирован за период:" << fromDate << "-" << toDate
             << "Записей:" << records.size();
}

void DateRangeReportDialog::onExportToFile() {
    if (m_currentReport.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Нет данных для экспорта. Сформируйте отчёт.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Сохранить отчёт",
        "report_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm") + ".txt",
        "Текстовые файлы (*.txt);;Все файлы (*)"
    );

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл:\n" + file.errorString());
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << m_currentReport;
    file.close();

    QMessageBox::information(this, "Успех",
        "Отчёт успешно экспортирован в файл:\n" + fileName);
    qDebug() << "Отчёт экспортирован в файл:" << fileName;
}

QString DateRangeReportDialog::generateReportText(const QVector<BaggageRecord>& records,
                                                  const QDateTime& from,
                                                  const QDateTime& to) {
    QString report;
    QTextStream stream(&report);

    // Заголовок отчёта
    stream << "========================================\n";
    stream << "   ОТЧЁТ ЗА ПЕРИОД ВРЕМЕНИ\n";
    stream << "========================================\n\n";

    stream << "Период: с " << from.toString("dd.MM.yyyy HH:mm")
           << " по " << to.toString("dd.MM.yyyy HH:mm") << "\n";
    stream << "Дата формирования: " << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss") << "\n\n";

    // Общая статистика
    stream << "----------------------------------------\n";
    stream << "ОБЩАЯ СТАТИСТИКА:\n";
    stream << "----------------------------------------\n";
    stream << "Количество записей: " << records.size() << "\n";

    // Подсчёт общего веса и количества вещей
    double totalWeight = 0.0;
    int totalItems = 0;
    QSet<QString> uniqueFlights;

    for (const auto& record : records) {
        totalWeight += record.getTotalWeight();
        totalItems += record.getItemWeights().size();
        uniqueFlights.insert(record.getFlightNumber());
    }

    stream << "Общий вес багажа: " << QString::number(totalWeight, 'f', 2) << " кг\n";
    stream << "Общее количество вещей: " << totalItems << "\n";
    stream << "Количество уникальных рейсов: " << uniqueFlights.size() << "\n\n";

    // Список уникальных рейсов
    stream << "----------------------------------------\n";
    stream << "СПИСОК РЕЙСОВ:\n";
    stream << "----------------------------------------\n";
    QStringList flightList = uniqueFlights.values();
    flightList.sort();
    for (const QString& flight : flightList) {
        // Подсчитываем количество записей для каждого рейса
        int count = 0;
        double flightWeight = 0.0;
        for (const auto& record : records) {
            if (record.getFlightNumber() == flight) {
                count++;
                flightWeight += record.getTotalWeight();
            }
        }
        stream << QString("  %1 - пассажиров: %2, вес: %3 кг\n")
                  .arg(flight, -10)
                  .arg(count, 3)
                  .arg(flightWeight, 0, 'f', 2);
    }
    stream << "\n";

    // Детальный список записей
    stream << "----------------------------------------\n";
    stream << "ДЕТАЛЬНЫЙ СПИСОК ЗАПИСЕЙ:\n";
    stream << "----------------------------------------\n";
    stream << QString("%1 | %2 | %3 | %4 | %5\n")
              .arg("№", -5)
              .arg("Рейс", -10)
              .arg("Пассажир", -30)
              .arg("Вещей", -7)
              .arg("Общий вес", -12);
    stream << QString("-").repeated(80) << "\n";

    for (int i = 0; i < records.size(); ++i) {
        const auto& record = records[i];
        stream << QString("%1 | %2 | %3 | %4 | %5 кг\n")
                  .arg(i + 1, -5)
                  .arg(record.getFlightNumber(), -10)
                  .arg(record.getPassengerName(), -30)
                  .arg(record.getItemWeights().size(), -7)
                  .arg(record.getTotalWeight(), -10, 'f', 2);
    }

    stream << "\n========================================\n";
    stream << "   КОНЕЦ ОТЧЁТА\n";
    stream << "========================================\n";

    return report;
}

void DateRangeReportDialog::showStatus(const QString& message, bool isError) {
    m_statusLabel->setText(message);

    if (isError) {
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    }
}
