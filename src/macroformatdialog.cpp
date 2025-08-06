#include "macroformatdialog.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

MacroFormatDialog::MacroFormatDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Insert Macro");

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setFormAlignment(Qt::AlignLeft);

    // Validador para campos restritos (label, key, field)
    QRegularExpression rx(R"([A-Za-z0-9_]+)");
    auto *validator = new QRegularExpressionValidator(rx, this);

    // Label
    labelEdit = new QLineEdit(this);
    labelEdit->setValidator(validator);
    formLayout->addRow("Label:", labelEdit);

    // Type
    typeCombo = new QComboBox(this);
    typeCombo->addItems({"string", "number", "date", "datetime", "bool", "combo"});
    formLayout->addRow("Type:", typeCombo);

    // Grupo de campos adicionais
    tableGroup = new QWidget(this);
    groupLayout = new QFormLayout(tableGroup);
    groupLayout->setLabelAlignment(Qt::AlignLeft);
    groupLayout->setFormAlignment(Qt::AlignLeft);

    // Table / Default value
    tableEdit = new QLineEdit(this); // Sem validador para permitir '%'
    tableLabel = new QLabel("Default value:", this);
    groupLayout->addRow(tableLabel, tableEdit);

    // Key field
    keyEdit = new QLineEdit(this);
    keyEdit->setValidator(validator);
    groupLayout->addRow("Key field:", keyEdit);

    // Field to show
    fieldEdit = new QLineEdit(this);
    fieldEdit->setValidator(validator);
    groupLayout->addRow("Field to show:", fieldEdit);

    tableGroup->setLayout(groupLayout);
    formLayout->addRow(tableGroup);

    // Exibe o campo Default value por padrão
    tableGroup->setVisible(true);
    keyEdit->setVisible(false);
    fieldEdit->setVisible(false);
    groupLayout->labelForField(keyEdit)->setVisible(false);
    groupLayout->labelForField(fieldEdit)->setVisible(false);

    connect(typeCombo, &QComboBox::currentTextChanged, this, &MacroFormatDialog::onTypeChanged);

    // Botões
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, [=]() {
        QString label = labelEdit->text().trimmed();
        QString type = typeCombo->currentText();

        if (label.isEmpty()) {
            reject();
            return;
        }

        QString defaultValue = tableEdit->text().trimmed();
        if (type == "string" && defaultValue.isEmpty()) {
            macroString = "~" + label;
        } else if (type == "combo") {
            QString table = tableEdit->text().trimmed();
            QString key = keyEdit->text().trimmed();
            QString field = fieldEdit->text().trimmed();
            macroString = "~" + label + "@combo~" + table + "~" + key + "~" + field;
        } else {
            if (defaultValue.isEmpty()) {
                macroString = "~" + label + "@" + type;
            } else {
                macroString = "~" + label + "@" + type + "~" + defaultValue;
            }
        }

        accept();
    });

    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setMinimumSize(400, 340);
}

void MacroFormatDialog::onTypeChanged(const QString &type)
{
    if (type == "combo") {
        tableGroup->setVisible(true);
        keyEdit->setVisible(true);
        fieldEdit->setVisible(true);
        tableLabel->setText("Table:");
        groupLayout->labelForField(keyEdit)->setVisible(true);
        groupLayout->labelForField(fieldEdit)->setVisible(true);
    } else {
        tableGroup->setVisible(true);
        keyEdit->setVisible(false);
        fieldEdit->setVisible(false);
        tableLabel->setText("Default value:");
        groupLayout->labelForField(keyEdit)->setVisible(false);
        groupLayout->labelForField(fieldEdit)->setVisible(false);
    }
}

QString MacroFormatDialog::resultMacro() const
{
    return macroString;
}
