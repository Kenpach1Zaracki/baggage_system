#include "DeleteByFlightDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

DeleteByFlightDialog::DeleteByFlightDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Удалить записи по номерам рейсов");
    createUI();
    resize(400, 250);
}

DeleteByFlightDialog::~DeleteByFlightDialog() {
}

void DeleteByFlightDialog::createUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Описание
    QLabel* descLabel = new QLabel(
        "Введите номера рейсов для удаления (каждый с новой строки):", this);
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);

    // Поле для ввода номеров рейсов
    m_flightNumbersEdit = new QTextEdit(this);
    m_flightNumbersEdit->setPlaceholderText("Например:\nSU1234\nAF567\nBA890");
    mainLayout->addWidget(m_flightNumbersEdit);

    // Предупреждение
    QLabel* warningLabel = new QLabel(
        "⚠ Будут удалены ВСЕ записи с указанными номерами рейсов!", this);
    warningLabel->setStyleSheet("color: red; font-weight: bold;");
    warningLabel->setWordWrap(true);
    mainLayout->addWidget(warningLabel);

    // Кнопки OK/Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &DeleteByFlightDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void DeleteByFlightDialog::onAccept() {
    if (validateInput()) {
        // Подтверждение удаления
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Подтверждение удаления",
            "Вы уверены, что хотите удалить записи с указанными номерами рейсов?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            accept();
        }
    }
}

bool DeleteByFlightDialog::validateInput() {
    QString text = m_flightNumbersEdit->toPlainText().trimmed();

    if (text.isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации",
            "Введите хотя бы один номер рейса!");
        m_flightNumbersEdit->setFocus();
        return false;
    }

    return true;
}

QStringList DeleteByFlightDialog::getFlightNumbers() const {
    QString text = m_flightNumbersEdit->toPlainText();
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    QStringList result;
    for (QString& line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            result.append(trimmed);
        }
    }

    return result;
}
