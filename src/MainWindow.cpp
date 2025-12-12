#include "MainWindow.h"
#include "AddRecordDialog.h"
#include "FilterDialog.h"
#include "DeleteByFlightDialog.h"
#include "ChangeItemsDialog.h"
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QHeaderView>
#include <QGroupBox>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_manager(std::make_unique<BaggageManager>()) {

    setWindowTitle("Система управления багажом пассажиров");
    resize(1000, 600);

    createMenus();
    createToolBar();
    createCentralWidget();
    updateStatusBar();
}

MainWindow::~MainWindow() {
}

void MainWindow::createMenus() {
    // Меню "Файл"
    QMenu* fileMenu = menuBar()->addMenu("Файл");

    QAction* newAction = fileMenu->addAction("Новый файл");
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);

    QAction* openAction = fileMenu->addAction("Открыть...");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);

    QAction* saveAction = fileMenu->addAction("Сохранить");
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);

    QAction* saveAsAction = fileMenu->addAction("Сохранить как...");
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAsFile);

    fileMenu->addSeparator();

    QAction* exitAction = fileMenu->addAction("Выход");
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);

    // Меню "Операции"
    QMenu* operationsMenu = menuBar()->addMenu("Операции");

    QAction* addAction = operationsMenu->addAction("Добавить запись");
    connect(addAction, &QAction::triggered, this, &MainWindow::onAddRecord);

    QAction* filterAction = operationsMenu->addAction("Фильтр пассажиров");
    connect(filterAction, &QAction::triggered, this, &MainWindow::onFilterRecords);

    QAction* deleteAction = operationsMenu->addAction("Удалить по рейсам");
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteByFlight);

    QAction* changeAction = operationsMenu->addAction("Изменить количество вещей");
    connect(changeAction, &QAction::triggered, this, &MainWindow::onChangeItemCount);

    // Меню "Помощь"
    QMenu* helpMenu = menuBar()->addMenu("Помощь");

    QAction* aboutAction = helpMenu->addAction("О программе");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createToolBar() {
    QToolBar* toolBar = addToolBar("Главная панель");
    toolBar->setMovable(false);

    toolBar->addAction("Открыть", this, &MainWindow::onOpenFile);
    toolBar->addAction("Сохранить", this, &MainWindow::onSaveFile);
    toolBar->addSeparator();
    toolBar->addAction("Добавить", this, &MainWindow::onAddRecord);
    toolBar->addAction("Удалить", this, &MainWindow::onDeleteByFlight);
}

void MainWindow::createCentralWidget() {
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Таблица для отображения данных
    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels(
        {"№ рейса", "Ф.И.О. пассажира", "Кол-во вещей", "Веса вещей (кг)", "Общий вес (кг)"});
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tableWidget->setAlternatingRowColors(true);

    mainLayout->addWidget(m_tableWidget);

    // Группа кнопок для 8 обязательных функций
    QGroupBox* functionsGroup = new QGroupBox("8 обязательных функций", this);
    QGridLayout* gridLayout = new QGridLayout(functionsGroup);

    m_btnCreateFile = new QPushButton("1. Создать файл", this);
    m_btnShowRecords = new QPushButton("2. Показать содержимое", this);
    m_btnFilter = new QPushButton("3. Фильтр (1 вещь, 20-30 кг)", this);
    m_btnCreateSummary = new QPushButton("4. Создать файл сводки", this);
    m_btnShowSummary = new QPushButton("5. Показать файл сводки", this);
    m_btnAddRecord = new QPushButton("6. Добавить запись", this);
    m_btnDeleteByFlight = new QPushButton("7. Удалить по рейсам", this);
    m_btnChangeItems = new QPushButton("8. Изменить кол-во вещей", this);

    gridLayout->addWidget(m_btnCreateFile, 0, 0);
    gridLayout->addWidget(m_btnShowRecords, 0, 1);
    gridLayout->addWidget(m_btnFilter, 0, 2);
    gridLayout->addWidget(m_btnCreateSummary, 0, 3);
    gridLayout->addWidget(m_btnShowSummary, 1, 0);
    gridLayout->addWidget(m_btnAddRecord, 1, 1);
    gridLayout->addWidget(m_btnDeleteByFlight, 1, 2);
    gridLayout->addWidget(m_btnChangeItems, 1, 3);

    connect(m_btnCreateFile, &QPushButton::clicked, this, &MainWindow::onCreateFile);
    connect(m_btnShowRecords, &QPushButton::clicked, this, &MainWindow::onShowRecords);
    connect(m_btnFilter, &QPushButton::clicked, this, &MainWindow::onFilterRecords);
    connect(m_btnCreateSummary, &QPushButton::clicked, this, &MainWindow::onCreateSummaryFile);
    connect(m_btnShowSummary, &QPushButton::clicked, this, &MainWindow::onShowSummaryFile);
    connect(m_btnAddRecord, &QPushButton::clicked, this, &MainWindow::onAddRecord);
    connect(m_btnDeleteByFlight, &QPushButton::clicked, this, &MainWindow::onDeleteByFlight);
    connect(m_btnChangeItems, &QPushButton::clicked, this, &MainWindow::onChangeItemCount);

    mainLayout->addWidget(functionsGroup);

    setCentralWidget(centralWidget);

    // Статусная строка
    m_statusLabel = new QLabel("Готово", this);
    statusBar()->addWidget(m_statusLabel);
}

void MainWindow::updateTable() {
    m_tableWidget->setRowCount(0);

    const QVector<BaggageRecord>& records = m_manager->getRecords();

    for (const BaggageRecord& record : records) {
        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        m_tableWidget->setItem(row, 0, new QTableWidgetItem(record.getFlightNumber()));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(record.getPassengerName()));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(record.getItemCount())));

        // Форматируем веса вещей
        QString weightsStr;
        for (double weight : record.getItemWeights()) {
            if (!weightsStr.isEmpty()) weightsStr += ", ";
            weightsStr += QString::number(weight, 'f', 2);
        }
        m_tableWidget->setItem(row, 3, new QTableWidgetItem(weightsStr));

        m_tableWidget->setItem(row, 4, new QTableWidgetItem(
            QString::number(record.getTotalWeight(), 'f', 2)));
    }

    updateStatusBar();
}

void MainWindow::updateStatusBar() {
    int count = m_manager->getRecordCount();
    QString status = QString("Записей: %1").arg(count);
    if (!m_currentFilename.isEmpty()) {
        status += QString(" | Файл: %1").arg(m_currentFilename);
    }
    m_statusLabel->setText(status);
}

// Функция 1: Создать файл
void MainWindow::onCreateFile() {
    QString filename = QFileDialog::getSaveFileName(this,
        "Создать новый файл базы данных",
        "",
        "Файлы базы данных (*.dat);;Все файлы (*)");

    if (filename.isEmpty()) {
        return;
    }

    if (m_manager->createFile(filename)) {
        m_currentFilename = filename;
        updateTable();
        QMessageBox::information(this, "Успех", "Файл успешно создан!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл!");
    }
}

// Функция 2: Показать содержимое файла
void MainWindow::onShowRecords() {
    updateTable();
    QMessageBox::information(this, "Содержимое файла",
        QString("Всего записей в базе: %1").arg(m_manager->getRecordCount()));
}

// Функция 3: Фильтр пассажиров с 1 вещью весом 20-30 кг
void MainWindow::onFilterRecords() {
    QVector<BaggageRecord> filtered = m_manager->filterPassengersWithSingleItem20_30kg();

    FilterDialog dialog(filtered, this);
    dialog.exec();
}

// Функция 4: Создать файл сводки
void MainWindow::onCreateSummaryFile() {
    if (m_manager->isEmpty()) {
        QMessageBox::warning(this, "Предупреждение",
            "База данных пуста! Добавьте записи перед созданием сводки.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this,
        "Сохранить файл сводки",
        "baggage_summary.txt",
        "Текстовые файлы (*.txt);;Все файлы (*)");

    if (filename.isEmpty()) {
        return;
    }

    if (m_manager->createSummaryFile(filename)) {
        QMessageBox::information(this, "Успех",
            QString("Файл сводки успешно создан:\n%1").arg(filename));
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл сводки!");
    }
}

// Функция 5: Показать файл сводки
void MainWindow::onShowSummaryFile() {
    QString filename = QFileDialog::getOpenFileName(this,
        "Открыть файл сводки",
        "",
        "Текстовые файлы (*.txt);;Все файлы (*)");

    if (filename.isEmpty()) {
        return;
    }

    // Открываем файл в стандартном текстовом редакторе
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(filename))) {
        QMessageBox::critical(this, "Ошибка",
            "Не удалось открыть файл!");
    }
}

// Функция 6: Добавить запись
void MainWindow::onAddRecord() {
    AddRecordDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        BaggageRecord record = dialog.getRecord();

        if (m_manager->addRecord(record)) {
            updateTable();
            QMessageBox::information(this, "Успех", "Запись успешно добавлена!");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить запись!");
        }
    }
}

// Функция 7: Удалить записи по номерам рейсов
void MainWindow::onDeleteByFlight() {
    if (m_manager->isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "База данных пуста!");
        return;
    }

    DeleteByFlightDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QStringList flightNumbers = dialog.getFlightNumbers();
        int deletedCount = m_manager->deleteRecordsByFlightNumbers(flightNumbers);

        updateTable();
        QMessageBox::information(this, "Результат",
            QString("Удалено записей: %1").arg(deletedCount));
    }
}

// Функция 8: Изменить количество вещей по ФИО
void MainWindow::onChangeItemCount() {
    if (m_manager->isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "База данных пуста!");
        return;
    }

    ChangeItemsDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted) {
        QString passengerName = dialog.getPassengerName();
        QVector<double> newWeights = dialog.getItemWeights();

        if (m_manager->changeItemCountByName(passengerName, newWeights)) {
            updateTable();
            QMessageBox::information(this, "Успех",
                QString("Количество вещей изменено для:\n%1").arg(passengerName));
        } else {
            QMessageBox::warning(this, "Предупреждение",
                QString("Пассажир не найден:\n%1").arg(passengerName));
        }
    }
}

void MainWindow::onNewFile() {
    onCreateFile();
}

void MainWindow::onOpenFile() {
    QString filename = QFileDialog::getOpenFileName(this,
        "Открыть файл базы данных",
        "",
        "Файлы базы данных (*.dat);;Все файлы (*)");

    if (filename.isEmpty()) {
        return;
    }

    if (m_manager->loadFromFile(filename)) {
        m_currentFilename = filename;
        updateTable();
        QMessageBox::information(this, "Успех", "Файл успешно загружен!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл!");
    }
}

void MainWindow::onSaveFile() {
    if (m_currentFilename.isEmpty()) {
        onSaveAsFile();
        return;
    }

    if (m_manager->saveToFile(m_currentFilename)) {
        QMessageBox::information(this, "Успех", "Файл успешно сохранен!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл!");
    }
}

void MainWindow::onSaveAsFile() {
    QString filename = QFileDialog::getSaveFileName(this,
        "Сохранить файл базы данных как",
        "",
        "Файлы базы данных (*.dat);;Все файлы (*)");

    if (filename.isEmpty()) {
        return;
    }

    if (m_manager->saveToFile(filename)) {
        m_currentFilename = filename;
        updateStatusBar();
        QMessageBox::information(this, "Успех", "Файл успешно сохранен!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл!");
    }
}

void MainWindow::onExit() {
    close();
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "О программе",
        "Система управления багажом пассажиров\n\n"
        "Курсовая работа по теме №96\n"
        "Разработка ПО для создания и обработки сведений о багаже пассажиров\n\n"
        "Реализованы все 8 обязательных функций:\n"
        "1. Создание файла\n"
        "2. Вывод содержимого\n"
        "3. Фильтр пассажиров (1 вещь, 20-30 кг)\n"
        "4. Создание файла сводки\n"
        "5. Просмотр файла сводки\n"
        "6. Добавление записи\n"
        "7. Удаление по номерам рейсов\n"
        "8. Изменение количества вещей\n\n"
        "Версия 1.0");
}
