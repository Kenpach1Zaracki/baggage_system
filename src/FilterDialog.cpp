#include "FilterDialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLabel>

FilterDialog::FilterDialog(const QVector<BaggageRecord>& records, QWidget* parent)
    : QDialog(parent), m_records(records) {
    setWindowTitle("Результаты фильтрации");
    createUI();
    populateTable();
    resize(600, 400);
}

FilterDialog::~FilterDialog() {
}

void FilterDialog::createUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Описание фильтра
    QLabel* descLabel = new QLabel(
        "Пассажиры с 1 вещью весом 20-30 кг:", this);
    descLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(descLabel);

    // Таблица результатов
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(3);
    m_tableWidget->setHorizontalHeaderLabels({"№ рейса", "Ф.И.О. пассажира", "Вес вещи (кг)"});

    // Настройка ширины столбцов
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_tableWidget->setColumnWidth(0, 120);  // № рейса

    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_tableWidget->setColumnWidth(2, 130);  // Вес вещи

    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);  // ФИО растягивается

    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);

    mainLayout->addWidget(m_tableWidget);

    // Информация о количестве найденных записей
    QLabel* countLabel = new QLabel(
        QString("Найдено записей: %1").arg(m_records.size()), this);
    mainLayout->addWidget(countLabel);

    // Кнопка закрытия
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

void FilterDialog::populateTable() {
    m_tableWidget->setRowCount(0);

    for (const BaggageRecord& record : m_records) {
        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        m_tableWidget->setItem(row, 0, new QTableWidgetItem(record.getFlightNumber()));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(record.getPassengerName()));

        // Вес первой (и единственной) вещи
        double weight = record.getItemWeights()[0];
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(weight, 'f', 2)));
    }
}
