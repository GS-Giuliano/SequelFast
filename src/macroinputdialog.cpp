#include "macroinputdialog.h"
#include "functions.h"

MacroInputDialog::MacroInputDialog(const QVector<MacroField> &fields, const QString &sql_host, const QString &sql_schema, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Data required");
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
            QString labelText = field.name;
            labelText.replace('_', ' ');
            auto *check = new QCheckBox(labelText, this);
            inputs[field.name] = check;
            inputWidget = check;
            layout->addRow("", inputWidget);
            continue;

        } else if (field.type == "combo") {
            auto *combo = new QComboBox(this);

            const QString &tableName   = field.table;
            const QString &keyField    = field.key;
            const QString &displayField = field.display;
            const QString &orderField  = field.order;
            qDebug() << "tableName: " << tableName;
            qDebug() << "keyField: " << keyField;
            qDebug() << "displayField: " << displayField;
            qDebug() << "orderField: " << orderField;

            if (tableName.isEmpty()) {
                combo->addItem("Erro: tabela não definida", QVariant());
                inputs[field.name] = combo;
                inputWidget = combo;
                continue;
            }

            connectMySQL(sql_host, parent);
            QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + sql_host);
            if (!db.open()) {
                combo->addItem("Erro: conexão BD", QVariant());
                inputs[field.name] = combo;
                inputWidget = combo;
                continue;
            }

            QSqlQuery query(db);
            QString queryStr = QString("SELECT * FROM %1.%2").arg(sql_schema, tableName);

            if (!query.exec(queryStr)) {
                combo->addItem("Erro: " + query.lastError().text(), QVariant());
                inputs[field.name] = combo;
                inputWidget = combo;
                continue;
            }

            QSqlRecord rec = query.record();

            int keyIndex = 0;
            int orderIndex = -1;
            QList<int> displayIndexes;

            // Chave primária
            if (!keyField.isEmpty())
                keyIndex = rec.indexOf(keyField);
            else
                keyIndex = 0;

            // Campo(s) de exibição
            if (!displayField.isEmpty()) {
                const QStringList parts = displayField.split(",", Qt::SkipEmptyParts);
                for (const QString &f : parts) {
                    int idx = rec.indexOf(f.trimmed());
                    if (idx >= 0)
                        displayIndexes << idx;
                }
            }

            // Heurística se campo de exibição não especificado
            if (displayIndexes.isEmpty()) {
                for (int i = 0; i < rec.count(); ++i) {
                    QString fname = rec.fieldName(i).toLower();
                    if (fname == "descricao" || fname == "descrição" || fname == "description" || fname == "nome") {
                        displayIndexes << i;
                        break;
                    }
                }
            }

            // Fallback absoluto
            if (displayIndexes.isEmpty())
                displayIndexes << (rec.count() > 1 ? 1 : 0);

            // Campo de ordenação
            QString effectiveOrderField = orderField;
            if (effectiveOrderField.isEmpty() && !displayIndexes.isEmpty())
                effectiveOrderField = rec.fieldName(displayIndexes.first());

            // Executa com ordenação
            queryStr = QString("SELECT * FROM %1.%2 ORDER BY %3").arg(sql_schema, tableName, effectiveOrderField);
            if (!query.exec(queryStr)) {
                combo->addItem("Erro: " + query.lastError().text(), QVariant());
            } else {
                while (query.next()) {
                    QString display;
                    for (int i = 0; i < displayIndexes.size(); ++i) {
                        if (i > 0) display += " ";
                        display += query.value(displayIndexes[i]).toString();
                    }
                    QVariant keyValue = query.value(keyIndex);
                    combo->addItem(display, keyValue);
                }
            }

            inputs[field.name] = combo;
            inputWidget = combo;

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
        else if (auto *w = qobject_cast<QComboBox *>(it.value()))
            values[it.key()] = w->currentData();
    }
    return values;
}
