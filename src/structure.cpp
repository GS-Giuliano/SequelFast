#include "structure.h"
#include "ui_structure.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QCompleter>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QMessageBox>

#include <functions.h>

#include <QStyledItemDelegate>
#include <QLineEdit>
#include <QRegularExpressionValidator>

extern QSqlDatabase dbMysql;


class RegexDelegateName : public QStyledItemDelegate {
public:
    RegexDelegateName(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &,
                          const QModelIndex &) const override
    {
        QLineEdit *editor = new QLineEdit(parent);
        QRegularExpression regex("^[a-zA-Z0-9_]*$",QRegularExpression::CaseInsensitiveOption);
        editor->setValidator(new QRegularExpressionValidator(regex, editor));
        return editor;
    }
};

class RegexDelegateType : public QStyledItemDelegate {
public:
    RegexDelegateType(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &,
                          const QModelIndex &) const override
    {
        QLineEdit *editor = new QLineEdit(parent);
        QRegularExpression regex(
            R"(^[a-zA-Z0-9_]+(\([0-9]+(,[0-9]+)?\))?( unsigned| zerofill| binary| CHARACTER SET [a-zA-Z0-9_]+| COLLATE [a-zA-Z0-9_]+)*$)",
            QRegularExpression::CaseInsensitiveOption);
        editor->setValidator(new QRegularExpressionValidator(regex, editor));

        // Força minúsculas conforme o usuário digita
        connect(editor, &QLineEdit::textChanged, editor, [editor]() {
            QString lower = editor->text().toLower();
            if (editor->text() != lower) {
                int cursorPos = editor->cursorPosition();
                editor->setText(lower);
                editor->setCursorPosition(cursorPos);
            }
        });

        // Autocompletar com sugestões de tipos SQL
        QStringList suggestions = {
            "int", "tinyint(1)", "bigint", "float", "double", "decimal(10,2)", "varchar(100)", "text", "date",
            "datetime", "tinyint", "mediumint", "smallint", "char(10)", "boolean"
        };

        QCompleter *completer = new QCompleter(suggestions, editor);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setFilterMode(Qt::MatchContains);
        editor->setCompleter(completer);
        return editor;
    }
};


class RegexDelegateYesNo : public QStyledItemDelegate {
public:
    RegexDelegateYesNo(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &,
                          const QModelIndex &) const override
    {
        QLineEdit *editor = new QLineEdit(parent);

        // Regex que permite apenas YES ou NO
        QRegularExpression regex(R"(^yes$|^no$)", QRegularExpression::CaseInsensitiveOption);
        auto *validator = new QRegularExpressionValidator(regex, editor);
        editor->setValidator(validator);

        // Converte para maiúsculas automaticamente
        connect(editor, &QLineEdit::textChanged, editor, [editor]() {
            QString upper = editor->text().toUpper();
            if (editor->text() != upper) {
                int pos = editor->cursorPosition();
                editor->setText(upper);
                editor->setCursorPosition(pos);
            }
        });
        // Autocompletar com YES e NO
        QStringList options = { "YES", "NO" };
        QCompleter *completer = new QCompleter(options, editor);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        editor->setCompleter(completer);
        return editor;
    }
};

Structure::Structure(QString &host, QString &schema, QString &table, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Structure)
{
    dbMysqlLocal = dbMysql;

    ui->setupUi(this);

    str_host = host;
    str_schema = schema;
    str_table = table;

    logCount = 0;

    this->setWindowTitle(schema + " • " + table);
    refresh_structure();

    modelLog = new QStandardItemModel(this);

    // Define os cabeçalhos das colunas
    modelLog->setHorizontalHeaderLabels(QStringList() << "Field" << "What" << "From" << "To");

    ui->tableLog->setModel(modelLog);
    ui->tableLog->resizeColumnsToContents();
    ui->tableLog->horizontalHeader()->setStretchLastSection(true);

    // Menu de contexto
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested,
            this, &Structure::show_context_menu);

}

Structure::~Structure()
{
    delete ui;
}

void Structure::refresh_structure()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    if (dbMysqlLocal.open())
    {
        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + str_host));
        QString queryStr = QString("USE %1;").arg(str_schema);
        if (query.exec(queryStr)) {

            QString queryStr = "DESCRIBE " + str_table;

            if (query.exec(queryStr)) {
                QStandardItemModel *model = new QStandardItemModel(this);

                // Define os cabeçalhos das colunas
                model->setHorizontalHeaderLabels(QStringList() << "Field" << "Type" << "Null" << "Key" << "Default" << "Extra");

                int row = 0;
                while (query.next()) {
                    model->setItem(row, 0, new QStandardItem(query.value("Field").toString()));
                    model->setItem(row, 1, new QStandardItem(query.value("Type").toString()));
                    model->setItem(row, 2, new QStandardItem(query.value("Null").toString()));
                    model->setItem(row, 3, new QStandardItem(query.value("Key").toString()));
                    model->setItem(row, 4, new QStandardItem(query.value("Default").toString()));
                    model->setItem(row, 5, new QStandardItem(query.value("Extra").toString()));
                    ++row;
                }

                ui->tableView->setModel(model);
                ui->tableView->resizeColumnsToContents();
                ui->tableView->horizontalHeader()->setStretchLastSection(true);

            } else {
                qCritical() << "Erro ao obter estrutura:" << query.lastError().text();
            }
            // Aplicar delegates dinamicamente após atualizar os dados
            int cols = ui->tableView->model()->columnCount();
            for (int col = 0; col < cols; ++col) {
                QString header = ui->tableView->model()->headerData(col, Qt::Horizontal).toString().toLower();
                if (header == "field") {
                    ui->tableView->setItemDelegateForColumn(col, new RegexDelegateName(this));
                } else if (header == "type") {
                    ui->tableView->setItemDelegateForColumn(col, new RegexDelegateType(this));
                } else if (header == "null") {
                    ui->tableView->setItemDelegateForColumn(col, new RegexDelegateYesNo(this));
                }
            }
            connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged,
                    this, [this](const QModelIndex &current, const QModelIndex &) {
                        if (current.isValid()) {
                            editIndex = current;
                            previousValue = ui->tableView->model()->data(current);
                        }
                    });

            connect(ui->tableView->model(), &QAbstractItemModel::dataChanged,
                    this, &Structure::on_tableData_changed);


            // Seleciona a primeira linha após carregar os dados
            if (ui->tableView->model()->rowCount() > 0) {
                QModelIndex firstIndex = ui->tableView->model()->index(0, 0);
                ui->tableView->selectionModel()->select(firstIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                ui->tableView->setCurrentIndex(firstIndex);
            }
        }
    } else {
        QMessageBox::warning(this, "Error", "Failed to connect to the database!");

    }

    QApplication::restoreOverrideCursor();
    QApplication::processEvents();

}

void Structure::on_tableData_changed(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (topLeft != bottomRight) return;

    QString value_new = ui->tableView->model()->data(topLeft).toString();
    QString value_old = previousValue.toString();

    QString fieldName;
    QString fieldType;

    QModelIndex indexSelecionado = ui->tableView->selectionModel()->currentIndex();

    if (indexSelecionado.isValid()) {
        int row_selected = indexSelecionado.row();
        QModelIndex index_col0 = ui->tableView->model()->index(row_selected, 0);  // coluna 1 (segunda coluna)
        QModelIndex index_col1 = ui->tableView->model()->index(row_selected, 1);  // coluna 1 (segunda coluna)

        fieldName = ui->tableView->model()->data(index_col0).toString();
        fieldType = ui->tableView->model()->data(index_col1).toString();
    }

    if (topLeft == editIndex)
    {
        if (value_new.trimmed().isEmpty()) {
            ui->tableView->model()->setData(topLeft, value_old);
            qDebug() << "Empty value!";
            return;
        }

        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + str_host));
        QString queryStr;
        QRegularExpression regex;

        switch(topLeft.column())
        {
            case 0: // nome do campo
                queryStr = "ALTER TABLE "+str_table+" CHANGE "+value_old+" "+value_new+" "+fieldType;
                qDebug() << queryStr;

                if (query.exec(queryStr)) {
                    ui->statusbar->showMessage("Success!");
                    log(fieldName, "Name changed", value_old, value_new);
                    previousValue = value_new;
                    editIndex = topLeft;
                    ui->tableView->resizeColumnsToContents();

                } else {
                    ui->tableView->model()->setData(topLeft, value_old);
                    ui->statusbar->showMessage("Edit error! "+query.lastError().text());
                    refresh_structure();
                }

                break;

            case 1: // tipo do campo
                value_new = value_new.toLower();
                queryStr = "ALTER TABLE "+str_table+" MODIFY COLUMN "+fieldName+" "+value_new;
                qDebug() << queryStr;

                if (query.exec(queryStr)) {
                    ui->statusbar->showMessage("Success!");
                    log(fieldName, "Type changed", value_old, value_new);
                    previousValue = value_new;
                    editIndex = topLeft;
                    ui->tableView->resizeColumnsToContents();

                } else {
                    ui->tableView->model()->setData(topLeft, value_old);
                    ui->statusbar->showMessage("Edit error! "+query.lastError().text());
                    refresh_structure();
                }

                break;
            case 2:
                value_new = value_new.toUpper();
                if (value_new == "YES")
                {
                    queryStr = "ALTER TABLE "+str_table+" MODIFY "+fieldName+ " " +fieldType+" NULL";
                } else {
                    queryStr = "ALTER TABLE "+str_table+" MODIFY "+fieldName+" "+fieldType+" NOT NULL";
                }
                qDebug() << queryStr;

                if (query.exec(queryStr)) {
                    ui->statusbar->showMessage("Success!");
                    log(fieldName, "Null changed", value_old.toUpper(), value_new.toUpper());
                    previousValue = value_new;
                    editIndex = topLeft;
                } else {
                    ui->tableView->model()->setData(topLeft, value_old);
                    ui->statusbar->showMessage("Edit error! "+query.lastError().text());
                    refresh_structure();
                }
                break;
            case 4: // default
                value_new = value_new.replace("'","");
                queryStr = "ALTER TABLE "+str_table+" MODIFY "+fieldName+ " " +fieldType+" DEFAULT '"+value_new+"'";
                qDebug() << queryStr;

                if (query.exec(queryStr)) {
                    ui->statusbar->showMessage("Success!");
                    log(fieldName, "Default changed", value_old, value_new);
                    previousValue = value_new;
                    editIndex = topLeft;
                    ui->tableView->resizeColumnsToContents();
                } else {
                    ui->tableView->model()->setData(topLeft, value_old);
                    ui->statusbar->showMessage("Edit error! "+query.lastError().text());
                    refresh_structure();
                }

                break;
        }
    }
}

void Structure::log(QString name, QString what, QString value_from, QString value_to)
{
    modelLog->setItem(logCount, 0, new QStandardItem(name));
    modelLog->setItem(logCount, 1, new QStandardItem(what));
    modelLog->setItem(logCount, 2, new QStandardItem(value_from));
    modelLog->setItem(logCount, 3, new QStandardItem(value_to));
    // if (logCount == 0)
    {
        ui->tableLog->resizeColumnsToContents();
        ui->tableLog->horizontalHeader()->setStretchLastSection(true);
    }
    logCount++;

    if (modelLog && modelLog->rowCount() > 0) {
        int ultimaLinha = modelLog->rowCount() - 1;
        QModelIndex index = modelLog->index(ultimaLinha, 0);

        ui->tableLog->scrollTo(index);  // opcional: rola até a linha
        ui->tableLog->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        ui->tableLog->setCurrentIndex(index);  // define o foco/seleção atual
    }
}

void Structure::add_new(const QModelIndex &index)
{

    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + str_host));
    QString queryStr;

    QString fieldName;
    QString fieldType;

    if (index.isValid()) {
        int row_selected = index.row();
        QModelIndex index_col0 = ui->tableView->model()->index(row_selected, 0);  // coluna 1 (segunda coluna)
        QModelIndex index_col1 = ui->tableView->model()->index(row_selected, 1);  // coluna 1 (segunda coluna)

        fieldName = ui->tableView->model()->data(index_col0).toString();
        fieldType = ui->tableView->model()->data(index_col1).toString();

        QString new_field_name = fieldName+"_new";

        queryStr = "ALTER TABLE "+str_table+" ADD COLUMN "+new_field_name+" "+fieldType+" AFTER "+fieldName;
        qDebug() << queryStr;

        if (query.exec(queryStr)) {
            ui->statusbar->showMessage("Success!");
            log(fieldName, "Add new", new_field_name, "");
            previousValue = new_field_name;
            editIndex = index;
            refresh_structure();

            QAbstractItemModel *model = ui->tableView->model();

            for (int row = 0; row < model->rowCount(); ++row) {
                QModelIndex indexFind = model->index(row, 0);  // primeira coluna
                QString valor = model->data(indexFind).toString();
                qDebug() << "busca: " << valor << "=" << new_field_name;
                if (valor.contains(new_field_name, Qt::CaseInsensitive)) {
                    qDebug() << "achou!!!!!!!!!!!";
                    ui->tableView->selectionModel()->select(indexFind, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                    ui->tableView->setCurrentIndex(indexFind);
                    ui->tableView->scrollTo(indexFind);
                    ui->tableView->edit(indexFind);
                    break;  // Para na primeira ocorrência
                }
            }
        } else {
            refresh_structure();
        }
    }

}

void Structure::delete_row()
{
    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + str_host));
    QString queryStr;

    QItemSelectionModel *selectionModel = ui->tableView->selectionModel();
    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();

    if (selectedIndexes.isEmpty()) {
        qDebug() << "Nenhuma célula selecionada.";
        return;
    }

    // Extrair todas as linhas únicas a partir das células selecionadas
    QSet<int> rowSet;
    for (const QModelIndex &index : selectedIndexes) {
        rowSet.insert(index.row());
    }

    // Converte para lista e ordena de forma decrescente
    QList<int> rows = rowSet.values();
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    QAbstractItemModel *model = ui->tableView->model();

    for (int row : rows) {
        QModelIndex indexField = model->index(row, 0);
        QVariant value = model->data(indexField);

        queryStr = "ALTER TABLE " + str_table + " DROP COLUMN " + value.toString();
        qDebug() << queryStr;

        if (query.exec(queryStr)) {
            ui->statusbar->showMessage("Removed!");
            log(value.toString(), "Remove", "", "");
            previousValue = value.toString();
            editIndex = indexField;
        } else {
            qWarning() << "Erro ao remover campo:" << query.lastError().text();
        }

        qDebug() << "Linha" << row << "| Valor da primeira coluna:" << value.toString();

        // Importante: remova a linha apenas no modelo visual, já que refresh_structure recarrega tudo
        model->removeRow(row);
    }

    refresh_structure();
}

void Structure::show_context_menu(const QPoint &pos)
{
    QModelIndex index = ui->tableView->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction *tableNew = menu.addAction("New after");
    QAction *tableRem = menu.addAction("Remove selected");
    QAction *tableUp = menu.addAction("Move up");
    QAction *tableDown = menu.addAction("Move down");

    QAction *selectedAction = menu.exec(ui->tableView->viewport()->mapToGlobal(pos));

    if (selectedAction == tableNew) {
        add_new(index);
    }
    else if (selectedAction == tableRem) {
        delete_row();
    }
    else if (selectedAction == tableUp) {
    }
    else if (selectedAction == tableDown) {
    }
}

void Structure::on_buttonUpdateFields_clicked()
{
    refresh_structure();
}

