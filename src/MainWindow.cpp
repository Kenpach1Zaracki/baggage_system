#include "MainWindow.h"
#include "AddRecordDialog.h"
#include "DateRangeReportDialog.h"
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
    : QMainWindow(parent), m_manager(std::make_unique<BaggageManager>()), m_isGuestMode(false), m_userRole("user") {

    setWindowTitle("Система управления багажом пассажиров");
    resize(1000, 600);

    createMenus();
    createToolBar();
    createCentralWidget();

    // КРИТИЧНО: Загружаем данные из БД при старте!
    updateTable();

    updateStatusBar();
}

MainWindow::~MainWindow() {
}

void MainWindow::setGuestMode(bool isGuest) {
    m_isGuestMode = isGuest;

    if (m_isGuestMode) {
        setWindowTitle("Система управления багажом пассажиров [ГОСТЕВОЙ РЕЖИМ - ТОЛЬКО ПРОСМОТР]");

        // Отключаем кнопки добавления/удаления/изменения
        if (m_btnAddRecord) m_btnAddRecord->setEnabled(false);
        if (m_btnDeleteByFlight) m_btnDeleteByFlight->setEnabled(false);
        if (m_btnChangeItems) m_btnChangeItems->setEnabled(false);
        if (m_btnCreateFile) m_btnCreateFile->setEnabled(false);

        // КРИТИЧНО: Отключаем пункты меню!
        QList<QAction*> allActions = findChildren<QAction*>();
        for (QAction* action : allActions) {
            QString text = action->text();
            
            // Отключаем действия изменения данных
            if (text.contains("Новый файл") ||
                text.contains("Сохранить") ||
                text.contains("Добавить") ||
                text.contains("Удалить") ||
                text.contains("Изменить")) {
                action->setEnabled(false);
            }
        }

        // КРИТИЧНО: Отключаем кнопки в toolbar!
        QList<QToolBar*> toolbars = findChildren<QToolBar*>();
        for (QToolBar* toolbar : toolbars) {
            QList<QAction*> actions = toolbar->actions();
            for (QAction* action : actions) {
                QString text = action->text();
                
                if (text.contains("Сохранить") ||
                    text.contains("Добавить") ||
                    text.contains("Удалить")) {
                    action->setEnabled(false);
                }
            }
        }

        QMessageBox::information(this, "Гостевой режим",
            "Вы вошли в гостевом режиме.\n\n"
            "Доступные функции:\n"
            "  - Просмотр всех записей\n"
            "  - Фильтрация данных\n"
            "  - Создание отчётов\n\n"
            "Операции добавления, удаления и изменения данных отключены.");
    }
}

void MainWindow::setUserRole(const QString& role) {
    m_userRole = role;
    qDebug() << "Установлена роль пользователя:" << role;
    setupPermissions();
}

void MainWindow::setupPermissions() {
    // Настройка прав доступа на основе роли пользователя
    bool isAdmin = (m_userRole == "admin");
    bool isUser = (m_userRole == "user");

    if (isAdmin) {
        setWindowTitle("Система управления багажом пассажиров [АДМИНИСТРАТОР]");
        // Админ имеет полный доступ ко всем функциям
        if (m_btnAddRecord) m_btnAddRecord->setEnabled(true);
        if (m_btnDeleteByFlight) m_btnDeleteByFlight->setEnabled(true);
        if (m_btnChangeItems) m_btnChangeItems->setEnabled(true);
        if (m_btnCreateFile) m_btnCreateFile->setEnabled(true);

    } else if (isUser) {
        setWindowTitle("Система управления багажом пассажиров [ПОЛЬЗОВАТЕЛЬ]");

        // Обычный пользователь может:
        // - Добавлять записи (CREATE)
        // - Просматривать (READ)
        // - Изменять СВОИ записи (UPDATE - ограниченно)
        // НО НЕ МОЖЕТ:
        // - Удалять записи (DELETE) - только админ
        // - Создавать новые файлы
        // - Очищать всю БД

        if (m_btnAddRecord) m_btnAddRecord->setEnabled(true);  // Может добавлять
        if (m_btnDeleteByFlight) m_btnDeleteByFlight->setEnabled(false);  // НЕ может удалять
        if (m_btnChangeItems) m_btnChangeItems->setEnabled(true);  // Может изменять
        if (m_btnCreateFile) m_btnCreateFile->setEnabled(false);  // НЕ может создавать файлы

        // Отключаем опасные операции в меню
        QList<QAction*> allActions = findChildren<QAction*>();
        for (QAction* action : allActions) {
            QString text = action->text();

            // Блокируем удаление для обычных пользователей
            if (text.contains("Удалить") || text.contains("Очистить")) {
                action->setEnabled(false);
            }
        }

        QMessageBox::information(this, "Права доступа",
            "Вы вошли как обычный пользователь.\n\n"
            "Доступные операции:\n"
            "  ✓ Просмотр записей\n"
            "  ✓ Добавление записей\n"
            "  ✓ Изменение записей\n"
            "  ✓ Создание отчётов\n\n"
            "Ограничения:\n"
            "  ✗ Удаление записей (только для администратора)\n"
            "  ✗ Создание новых файлов",
            QMessageBox::Information);
    }
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
    {"№ РЕЙСА", "Ф.И.О. ПАССАЖИРА", "КОЛ-ВО", "ВЕСА (КГ)", "ОБЩИЙ ВЕС (КГ)"});

// ========== НАСТРОЙКА ШИРИНЫ КОЛОНОК ==========
// Фиксированная ширина для узких колонок
m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
m_tableWidget->setColumnWidth(0, 120);  // № рейса

m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
m_tableWidget->setColumnWidth(2, 100);   // Кол-во вещей

m_tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
m_tableWidget->setColumnWidth(4, 150);  // Общий вес

// Растягиваем длинные колонки пропорционально
m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);  // ФИО
m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);  // Веса
// ============================================

// Высота строк и заголовка
m_tableWidget->verticalHeader()->setDefaultSectionSize(45);
m_tableWidget->horizontalHeader()->setMinimumHeight(50);

// Остальные настройки
m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
m_tableWidget->setAlternatingRowColors(true);

// Панель кнопок управления
QHBoxLayout* buttonLayout = new QHBoxLayout();

m_btnCreateFile = new QPushButton("Создать файл", this);
connect(m_btnCreateFile, &QPushButton::clicked, this, &MainWindow::onCreateFile);

QPushButton* btnShowRecords = new QPushButton("Показать записи", this);
connect(btnShowRecords, &QPushButton::clicked, this, &MainWindow::onShowRecords);

QPushButton* btnFilter = new QPushButton("Фильтр (1 вещь 20-30кг)", this);
connect(btnFilter, &QPushButton::clicked, this, &MainWindow::onFilterRecords);

QPushButton* btnCreateSummary = new QPushButton("Создать сводку", this);
connect(btnCreateSummary, &QPushButton::clicked, this, &MainWindow::onCreateSummaryFile);

QPushButton* btnShowSummary = new QPushButton("Показать сводку", this);
connect(btnShowSummary, &QPushButton::clicked, this, &MainWindow::onShowSummaryFile);

m_btnAddRecord = new QPushButton("Добавить запись", this);
connect(m_btnAddRecord, &QPushButton::clicked, this, &MainWindow::onAddRecord);

m_btnDeleteByFlight = new QPushButton("Удалить по рейсам", this);
connect(m_btnDeleteByFlight, &QPushButton::clicked, this, &MainWindow::onDeleteByFlight);

m_btnChangeItems = new QPushButton("Изменить вещи", this);
connect(m_btnChangeItems, &QPushButton::clicked, this, &MainWindow::onChangeItemCount);

QPushButton* btnDateReport = new QPushButton("Отчёт за период", this);
connect(btnDateReport, &QPushButton::clicked, this, &MainWindow::onGenerateDateReport);

buttonLayout->addWidget(m_btnCreateFile);
buttonLayout->addWidget(btnShowRecords);
buttonLayout->addWidget(btnFilter);
buttonLayout->addWidget(btnCreateSummary);
buttonLayout->addWidget(btnShowSummary);
buttonLayout->addWidget(m_btnAddRecord);
buttonLayout->addWidget(m_btnDeleteByFlight);
buttonLayout->addWidget(m_btnChangeItems);
buttonLayout->addWidget(btnDateReport);

mainLayout->addLayout(buttonLayout);
mainLayout->addWidget(m_tableWidget);

// Создаём status bar и status label (только один раз в конструкторе)
m_statusBar = statusBar();
m_statusLabel = new QLabel("Готов к работе", this);
m_statusBar->addPermanentWidget(m_statusLabel);

setCentralWidget(centralWidget);

}

void MainWindow::updateTable() {
    if (!m_tableWidget) {
        qWarning() << "Table widget is null!";
        return;
    }

    if (!m_manager) {
        qWarning() << "BaggageManager is null!";
        return;
    }

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
    if (!m_statusLabel) {
        qWarning() << "Status label is null!";
        return;
    }

    if (!m_manager) {
        qWarning() << "BaggageManager is null!";
        return;
    }

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
void MainWindow::onGenerateDateReport() {
    DateRangeReportDialog dialog(this);
    dialog.exec();
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
