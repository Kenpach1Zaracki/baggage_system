#include "LoginDialog.h"
#include "DatabaseManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , m_loginResult(Cancelled)
    , m_failedAttempts(0)
{
    setupUI();
    setWindowTitle("Авторизация - Система управления багажом");
    setModal(true);
    setMinimumWidth(400);
}

LoginDialog::~LoginDialog() {
}

void LoginDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Заголовок
    QLabel* titleLabel = new QLabel("Система управления багажом", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    mainLayout->addSpacing(20);

    // Форма ввода
    QFormLayout* formLayout = new QFormLayout();

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("Минимум 3 символа");
    formLayout->addRow("Логин:", m_usernameEdit);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Минимум 4 символа");
    formLayout->addRow("Пароль:", m_passwordEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(10);

    // Статусная метка
    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addSpacing(10);

    // Кнопки
    QVBoxLayout* buttonLayout = new QVBoxLayout();

    m_loginButton = new QPushButton("Войти", this);
    m_loginButton->setDefault(true);
    buttonLayout->addWidget(m_loginButton);

    m_registerButton = new QPushButton("Регистрация", this);
    buttonLayout->addWidget(m_registerButton);

    m_guestButton = new QPushButton("Гостевой режим", this);
    buttonLayout->addWidget(m_guestButton);

    mainLayout->addLayout(buttonLayout);

    // Подсказка
    QLabel* hintLabel = new QLabel(
        "Для первого входа используйте:\n"
        "Логин: admin, Пароль: admin\n"
        "или войдите как гость (только просмотр)",
        this
    );
    hintLabel->setStyleSheet("color: gray; font-size: 10pt;");
    hintLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(hintLabel);

    // Подключение сигналов
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(m_guestButton, &QPushButton::clicked, this, &LoginDialog::onGuestClicked);

    // Enter нажимает кнопку входа
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (!validateInput(username, password)) {
        return;
    }

    if (authenticateUser(username, password)) {
        m_username = username;
        m_userRole = getUserRole(username);  // Получаем роль пользователя
        m_loginResult = LoggedIn;
        m_failedAttempts = 0;  // Сброс счетчика при успешном входе
        showStatus("Вход выполнен успешно!", false);
        qDebug() << "Успешная аутентификация. Роль:" << m_userRole;  // НЕ логируем username
        accept();
    } else {
        m_failedAttempts++;
        int attemptsLeft = MAX_LOGIN_ATTEMPTS - m_failedAttempts;

        if (attemptsLeft > 0) {
            showStatus(QString("Неверный логин или пароль! Осталось попыток: %1").arg(attemptsLeft), true);
            qWarning() << "Неудачная попытка аутентификации. Попытка" << m_failedAttempts << "из" << MAX_LOGIN_ATTEMPTS;  // НЕ логируем username
        } else {
            QMessageBox::critical(this, "Доступ заблокирован",
                "Превышено максимальное количество попыток входа!\n"
                "Приложение будет закрыто из соображений безопасности.");
            qWarning() << "Превышено количество попыток входа. Закрытие приложения.";
            reject();
        }
    }
}

void LoginDialog::onRegisterClicked() {
    QString username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();

    if (!validateInput(username, password)) {
        return;
    }

    if (registerUser(username, password)) {
        showStatus("Регистрация успешна! Теперь можете войти.", false);
        qDebug() << "Новый пользователь успешно зарегистрирован";  // НЕ логируем username
        QMessageBox::information(this, "Успех",
            "Регистрация успешна!\n"
            "Теперь вы можете войти.");
    } else {
        showStatus("Ошибка регистрации! Возможно, пользователь уже существует.", true);
        qWarning() << "Ошибка регистрации: пользователь уже существует или ошибка БД";
    }
}

void LoginDialog::onGuestClicked() {
    m_username = "guest";
    m_loginResult = GuestMode;
    qDebug() << "Вход в гостевом режиме";
    accept();
}

bool LoginDialog::authenticateUser(const QString& username, const QString& password) {
    QSqlDatabase& db = DatabaseManager::instance().getDatabase();

    if (!db.isOpen()) {
        qWarning() << "База данных не подключена";
        return false;
    }

    QString hashedPassword = hashPassword(password);

    // Защита от SQL-инъекций через использование QSqlQuery с позиционными параметрами
    QSqlQuery query(db);

    // Используем exec() с prepare() и параметрами
    if (!query.prepare("SELECT id, username, role FROM users WHERE username = ? AND password_hash = ?")) {
        qWarning() << "Ошибка подготовки запроса:" << query.lastError().text();
        return false;
    }

    query.bindValue(0, username);
    query.bindValue(1, hashedPassword);

    if (!query.exec()) {
        qWarning() << "Ошибка выполнения запроса аутентификации:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return true;
    }

    return false;
}

QString LoginDialog::getUserRole(const QString& username) {
    QSqlDatabase& db = DatabaseManager::instance().getDatabase();

    if (!db.isOpen()) {
        qWarning() << "База данных не подключена при получении роли";
        return "user";  // По умолчанию обычный пользователь
    }

    QSqlQuery query(db);
    if (!query.prepare("SELECT role FROM users WHERE username = ?")) {
        qWarning() << "Ошибка подготовки запроса роли:" << query.lastError().text();
        return "user";
    }

    query.bindValue(0, username);

    if (!query.exec()) {
        qWarning() << "Ошибка выполнения запроса роли:" << query.lastError().text();
        return "user";
    }

    if (query.next()) {
        QString role = query.value(0).toString();
        return role.isEmpty() ? "user" : role;
    }

    return "user";
}

bool LoginDialog::registerUser(const QString& username, const QString& password) {
    QSqlDatabase& db = DatabaseManager::instance().getDatabase();

    if (!db.isOpen()) {
        qWarning() << "База данных не подключена";
        return false;
    }

    QString hashedPassword = hashPassword(password);

    // Проверка, существует ли пользователь
    QSqlQuery checkQuery(db);
    if (!checkQuery.prepare("SELECT id FROM users WHERE username = ?")) {
        qWarning() << "Ошибка подготовки запроса проверки:" << checkQuery.lastError().text();
        return false;
    }
    checkQuery.bindValue(0, username);

    if (!checkQuery.exec()) {
        qWarning() << "Ошибка проверки существования пользователя:" << checkQuery.lastError().text();
        return false;
    }

    if (checkQuery.next()) {
        qWarning() << "Пользователь" << username << "уже существует";
        return false;
    }

    // Регистрация нового пользователя
    QSqlQuery insertQuery(db);
    if (!insertQuery.prepare("INSERT INTO users (username, password_hash, role) VALUES (?, ?, 'user')")) {
        qWarning() << "Ошибка подготовки запроса регистрации:" << insertQuery.lastError().text();
        return false;
    }
    insertQuery.bindValue(0, username);
    insertQuery.bindValue(1, hashedPassword);

    if (!insertQuery.exec()) {
        qWarning() << "Ошибка регистрации пользователя:" << insertQuery.lastError().text();
        return false;
    }

    return true;
}

QString LoginDialog::hashPassword(const QString& password) {
    // SHA-256 хеширование пароля
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hash = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool LoginDialog::validateInput(const QString& username, const QString& password) {
    if (username.length() < 3) {
        showStatus("Логин должен содержать минимум 3 символа!", true);
        m_usernameEdit->setFocus();
        return false;
    }

    if (password.length() < 4) {
        showStatus("Пароль должен содержать минимум 4 символа!", true);
        m_passwordEdit->setFocus();
        return false;
    }

    return true;
}

void LoginDialog::showStatus(const QString& message, bool isError) {
    m_statusLabel->setText(message);

    if (isError) {
        m_statusLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        m_statusLabel->setStyleSheet("color: green; font-weight: bold;");
    }
}
