#include "MainWindow.h"
#include "DatabaseManager.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Установка информации о приложении
    QApplication::setApplicationName("Система управления багажом");
    QApplication::setApplicationVersion("2.0");
    QApplication::setOrganizationName("Курсовая работа");

    // Установка русской локали
    QLocale::setDefault(QLocale(QLocale::Russian, QLocale::Russia));

    // Подключение к PostgreSQL
    QString dbHost = qEnvironmentVariable("DB_HOST", "localhost");
    int dbPort = qEnvironmentVariable("DB_PORT", "5432").toInt();
    QString dbName = qEnvironmentVariable("DB_NAME", "baggage_db");
    QString dbUser = qEnvironmentVariable("DB_USER", "postgres");
    QString dbPassword = qEnvironmentVariable("DB_PASSWORD", "postgres");

    DatabaseManager& dbManager = DatabaseManager::instance();

    if (!dbManager.connectToDatabase(dbHost, dbPort, dbName, dbUser, dbPassword)) {
        QMessageBox::critical(nullptr, "Ошибка подключения к БД",
                             "Не удалось подключиться к PostgreSQL:\n" +
                             dbManager.getLastError() + "\n\n" +
                             "Параметры подключения:\n" +
                             "Host: " + dbHost + ":" + QString::number(dbPort) + "\n" +
                             "Database: " + dbName + "\n" +
                             "User: " + dbUser);
        return 1;
    }

    // Создаем таблицу, если её нет
    if (!dbManager.createTable()) {
        QMessageBox::critical(nullptr, "Ошибка инициализации БД",
                             "Не удалось создать таблицу:\n" +
                             dbManager.getLastError());
        return 1;
    }

    qDebug() << "✓ Подключение к PostgreSQL успешно";

    // Загрузка и применение стилей
    QFile styleFile(":/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QString::fromUtf8(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        qDebug() << "✓ Стили применены успешно";
        styleFile.close();
    } else {
        qWarning() << "⚠ Не удалось загрузить файл стилей";
    }

    // Показ диалога авторизации (ТЗ п. 1.2.8.4.2)
    LoginDialog loginDialog;
    if (loginDialog.exec() != QDialog::Accepted) {
        qDebug() << "Авторизация отменена, выход из приложения";
        return 0;
    }

    // Создание и показ главного окна
    MainWindow mainWindow;

    // Установка гостевого режима, если пользователь вошёл как гость
    if (loginDialog.isGuestMode()) {
        mainWindow.setGuestMode(true);
        qDebug() << "Запуск в гостевом режиме (только просмотр)";
    } else {
        // Устанавливаем роль пользователя для RBAC
        QString userRole = loginDialog.getUserRole();
        mainWindow.setUserRole(userRole);
        qDebug() << "Пользователь успешно аутентифицирован. Роль:" << userRole;
    }

    mainWindow.show();

    int result = app.exec();

    // Отключение от БД при выходе
    dbManager.disconnectFromDatabase();

    return result;
}
