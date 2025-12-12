#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "BaggageManager.h"

/**
 * @brief Главное окно приложения для управления багажом пассажиров
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Меню "Файл"
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onExit();

    // Функции по заданию
    void onCreateFile();           // Функция 1
    void onShowRecords();          // Функция 2
    void onFilterRecords();        // Функция 3
    void onCreateSummaryFile();    // Функция 4
    void onShowSummaryFile();      // Функция 5
    void onAddRecord();            // Функция 6
    void onDeleteByFlight();       // Функция 7
    void onChangeItemCount();      // Функция 8

    // Вспомогательные слоты
    void onAbout();

private:
    void createMenus();
    void createToolBar();
    void createCentralWidget();
    void updateTable();
    void updateStatusBar();

    // GUI компоненты
    QTableWidget* m_tableWidget;
    QStatusBar* m_statusBar;
    QLabel* m_statusLabel;

    // Кнопки для операций
    QPushButton* m_btnCreateFile;
    QPushButton* m_btnShowRecords;
    QPushButton* m_btnFilter;
    QPushButton* m_btnCreateSummary;
    QPushButton* m_btnShowSummary;
    QPushButton* m_btnAddRecord;
    QPushButton* m_btnDeleteByFlight;
    QPushButton* m_btnChangeItems;

    // Менеджер данных
    std::unique_ptr<BaggageManager> m_manager;
    QString m_currentFilename;
};

#endif // MAINWINDOW_H
