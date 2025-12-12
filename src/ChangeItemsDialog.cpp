#include "ChangeItemsDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QGroupBox>
#include <QLabel>

ChangeItemsDialog::ChangeItemsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle("Изменить количество вещей");
    createUI();
    resize(400, 350);
}

ChangeItemsDialog::~ChangeItemsDialog() {
}

void ChangeItemsDialog::createUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Описание
    QLabel* descLabel = new QLabel(
        "Будут изменены ВСЕ записи с указанным Ф.И.О.", this);
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(descLabel);

    // Поле для ввода ФИО
    QFormLayout* formLayout = new QFormLayout();

    m_passengerNameEdit = new QLineEdit(this);
    m_passengerNameEdit->setPlaceholderText("Иванов И.И.");
    formLayout->addRow("Ф.И.О. пассажира:", m_passengerNameEdit);

    m_itemCountSpinBox = new QSpinBox(this);
    m_itemCountSpinBox->setMinimum(1);
    m_itemCountSpinBox->setMaximum(MAX_ITEMS);
    m_itemCountSpinBox->setValue(1);
    formLayout->addRow("Новое количество вещей:", m_itemCountSpinBox);

    mainLayout->addLayout(formLayout);

    // Группа для ввода весов
    QGroupBox* weightsGroup = new QGroupBox("Новые веса вещей (кг)", this);
    QFormLayout* weightsLayout = new QFormLayout(weightsGroup);

    // Создаем поля для ввода весов
    for (int i = 0; i < MAX_ITEMS; ++i) {
        QLabel* label = new QLabel(QString("Вещь %1:").arg(i + 1), this);
        QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
        spinBox->setMinimum(0.1);
        spinBox->setMaximum(100.0);
        spinBox->setValue(10.0);
        spinBox->setDecimals(2);
        spinBox->setSuffix(" кг");

        m_weightLabels.append(label);
        m_weightSpinBoxes.append(spinBox);
        weightsLayout->addRow(label, spinBox);

        // Изначально видны только поля для первой вещи
        if (i > 0) {
            label->setVisible(false);
            spinBox->setVisible(false);
        }
    }

    mainLayout->addWidget(weightsGroup);

    // Кнопки OK/Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ChangeItemsDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // При изменении количества вещей - показываем/скрываем поля
    connect(m_itemCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ChangeItemsDialog::onItemCountChanged);
}

void ChangeItemsDialog::onItemCountChanged(int count) {
    // Показываем только нужное количество полей для весов
    for (int i = 0; i < MAX_ITEMS; ++i) {
        bool visible = (i < count);
        m_weightLabels[i]->setVisible(visible);
        m_weightSpinBoxes[i]->setVisible(visible);
    }
}

void ChangeItemsDialog::onAccept() {
    if (validateInput()) {
        accept();
    }
}

bool ChangeItemsDialog::validateInput() {
    // Проверка ФИО
    if (m_passengerNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка валидации",
            "Введите Ф.И.О. пассажира!");
        m_passengerNameEdit->setFocus();
        return false;
    }

    // Проверка весов
    int itemCount = m_itemCountSpinBox->value();
    for (int i = 0; i < itemCount; ++i) {
        if (m_weightSpinBoxes[i]->value() <= 0) {
            QMessageBox::warning(this, "Ошибка валидации",
                QString("Вес вещи %1 должен быть больше 0!").arg(i + 1));
            m_weightSpinBoxes[i]->setFocus();
            return false;
        }
    }

    return true;
}

QString ChangeItemsDialog::getPassengerName() const {
    return m_passengerNameEdit->text().trimmed();
}

QVector<double> ChangeItemsDialog::getItemWeights() const {
    QVector<double> weights;
    int itemCount = m_itemCountSpinBox->value();
    for (int i = 0; i < itemCount; ++i) {
        weights.append(m_weightSpinBoxes[i]->value());
    }
    return weights;
}
