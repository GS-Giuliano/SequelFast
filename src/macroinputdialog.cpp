#include "macroinputdialog.h"

MacroInputDialog::MacroInputDialog(const QVector<MacroField> &fields, QWidget *parent)
    : QDialog(parent)
{
    QFormLayout *layout = new QFormLayout(this);

    for (const auto &field : fields) {
        QWidget *inputWidget = nullptr;

        if (field.type == "date") {
            auto *dateEdit = new QDateEdit(QDate::currentDate(), this);
            dateEdit->setCalendarPopup(true);
            inputs[field.name] = dateEdit;
            inputWidget = dateEdit;

        } else if (field.type == "datetime") {
            auto *dtEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
            dtEdit->setCalendarPopup(true);
            inputs[field.name] = dtEdit;
            inputWidget = dtEdit;

        } else if (field.type == "number") {
            auto *spin = new QDoubleSpinBox(this);
            spin->setMaximum(1e9);
            inputs[field.name] = spin;
            inputWidget = spin;

        } else if (field.type == "bool") {
            auto *check = new QCheckBox(this);
            inputs[field.name] = check;
            inputWidget = check;

        } else {
            auto *lineEdit = new QLineEdit(this);
            inputs[field.name] = lineEdit;
            inputWidget = lineEdit;
        }

        QString labelText = field.name;
        labelText.replace('_', ' ');
        layout->addRow(labelText, inputWidget);
    }

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QMap<QString, QVariant> MacroInputDialog::getValues() const {
    QMap<QString, QVariant> values;
    for (auto it = inputs.begin(); it != inputs.end(); ++it) {
        if (auto *w = qobject_cast<QLineEdit *>(it.value()))
            values[it.key()] = w->text();
        else if (auto *w = qobject_cast<QDoubleSpinBox *>(it.value()))
            values[it.key()] = w->value();
        else if (auto *w = qobject_cast<QDateEdit *>(it.value()))
            values[it.key()] = w->date().toString("yyyy-MM-dd");
        else if (auto *w = qobject_cast<QDateTimeEdit *>(it.value()))
            values[it.key()] = w->dateTime().toString(Qt::ISODate);
        else if (auto *w = qobject_cast<QCheckBox *>(it.value()))
            values[it.key()] = w->isChecked() ? 1 : 0;
    }
    return values;
}
