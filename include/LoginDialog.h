#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    enum LoginResult {
        Cancelled = 0,
        LoggedIn = 1,
        GuestMode = 2
    };

    LoginResult getLoginResult() const { return m_loginResult; }
    QString getUsername() const { return m_username; }
    QString getUserRole() const { return m_userRole; }
    bool isGuestMode() const { return m_loginResult == GuestMode; }
    bool isAdmin() const { return m_userRole == "admin"; }

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onGuestClicked();

private:
    // UI элементы
    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QPushButton* m_loginButton;
    QPushButton* m_registerButton;
    QPushButton* m_guestButton;
    QLabel* m_statusLabel;

    // Состояние
    LoginResult m_loginResult;
    QString m_username;
    QString m_userRole; 
    int m_failedAttempts;
    static constexpr int MAX_LOGIN_ATTEMPTS = 3;

    // Методы для работы с БД
    bool authenticateUser(const QString& username, const QString& password);
    bool registerUser(const QString& username, const QString& password);
    QString getUserRole(const QString& username);
    QString hashPassword(const QString& password);

    // Валидация
    bool validateInput(const QString& username, const QString& password);

    // UI методы
    void setupUI();
    void showStatus(const QString& message, bool isError = false);
};

#endif // LOGINDIALOG_H
