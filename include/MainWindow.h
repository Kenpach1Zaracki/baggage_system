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

    // Установка гостевого режима (ТЗ п. 1.2.8.4.2)
    void setGuestMode(bool isGuest);

    // Установка роли пользователя (RBAC - Role-Based Access Control)
    void setUserRole(const QString& role);

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
    void onGenerateDateReport();

    // Вспомогательные слоты
    void onAbout();

private:
    void createMenus();
    void createToolBar();
    void createCentralWidget();
    void updateTable();
    void updateStatusBar();
    void setupPermissions();  // Настройка прав доступа на основе роли

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

    // Гостевой режим и роли
    bool m_isGuestMode;
    QString m_userRole;  // Роль пользователя (admin/user/guest)
    QString m_username;  // Имя текущего пользователя
};

#endif // MAINWINDOW_H
