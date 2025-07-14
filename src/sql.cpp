#include "sql.h"
#include "ui_sql.h"
#include "sqlhighlighter.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QStyledItemDelegate>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QMessageBox>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QToolButton>
#include <QPushButton>
#include <QStandardItemModel>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <functions.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;
extern QString currentTheme;

extern int pref_sql_limit;
extern int pref_table_row_height;
extern int pref_table_font_size;
extern int pref_sql_font_size;

class CustomDelegate : public QStyledItemDelegate {
public:
    CustomDelegate(QObject *parent, const QSqlRecord &record)
        : QStyledItemDelegate(parent), currentRecord(record) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override {
        QSqlField field = currentRecord.field(index.column());
        QVariant::Type type = field.type();
        int fieldSize = field.length();

        if (type == QVariant::Date) {
            auto *editor = new QDateEdit(parent);
            editor->setCalendarPopup(true);
            editor->setDisplayFormat("yyyy-mm-dd");
            // editor->setDisplayFormat("dd-mm-yyyy");
            return editor;
        }

        if (type == QVariant::DateTime || type == QVariant::Time) {
            auto *editor = new QDateTimeEdit(parent);
            editor->setCalendarPopup(true);
            editor->setDisplayFormat("yyyy-mm-dd HH:mm:ss");
            // editor->setDisplayFormat("dd-mm-yyyy HH:mm:ss");
            return editor;
        }

        if (type == QVariant::String && fieldSize > 128) {
            auto *editor = new QPlainTextEdit(parent);
            editor->setMinimumHeight(80);
            return editor;
        }

        if (type == QVariant::Int || type == QVariant::UInt ||
            type == QVariant::LongLong || type == QVariant::ULongLong) {
            auto *editor = new QLineEdit(parent);
            editor->setValidator(new QIntValidator(editor));
            return editor;
        }

        if (type == QVariant::Double) {
            auto *editor = new QLineEdit(parent);
            auto *validator = new QDoubleValidator(editor);
            validator->setNotation(QDoubleValidator::StandardNotation);
            validator->setDecimals(10); // defina aqui a precisão desejada
            editor->setValidator(validator);
            return editor;
        }

        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        QSqlField field = currentRecord.field(index.column());
        QVariant::Type type = field.type();
        int fieldSize = field.length();

        if (type == QVariant::Date) {
            auto *dateEditor = qobject_cast<QDateEdit *>(editor);
            dateEditor->setDate(QDate::fromString(value, "yyyy-MM-dd"));
            return;
        }

        if (type == QVariant::DateTime || type == QVariant::Time) {
            auto *dtEditor = qobject_cast<QDateTimeEdit *>(editor);
            dtEditor->setDateTime(QDateTime::fromString(value, "yyyy-MM-dd HH:mm:ss"));
            return;
        }

        if (type == QVariant::String && fieldSize > 128) {
            auto *textEditor = qobject_cast<QPlainTextEdit *>(editor);
            textEditor->setPlainText(value);
            return;
        }

        if (auto *lineEdit = qobject_cast<QLineEdit *>(editor)) {
            lineEdit->setText(value);
            return;
        }

        QStyledItemDelegate::setEditorData(editor, index);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override {
        QSqlField field = currentRecord.field(index.column());
        QVariant::Type type = field.type();
        int fieldSize = field.length();

        if (type == QVariant::Date) {
            auto *dateEditor = qobject_cast<QDateEdit *>(editor);
            model->setData(index, dateEditor->date().toString("yyyy-MM-dd"));
            return;
        }

        if (type == QVariant::DateTime || type == QVariant::Time) {
            auto *dtEditor = qobject_cast<QDateTimeEdit *>(editor);
            model->setData(index, dtEditor->dateTime().toString("yyyy-MM-dd HH:mm:ss"));
            return;
        }

        if (type == QVariant::String && fieldSize > 128) {
            auto *textEditor = qobject_cast<QPlainTextEdit *>(editor);
            model->setData(index, textEditor->toPlainText());
            return;
        }

        if (auto *lineEdit = qobject_cast<QLineEdit *>(editor)) {
            model->setData(index, lineEdit->text());
            return;
        }

        QStyledItemDelegate::setModelData(editor, model, index);
    }

private:
    QSqlRecord currentRecord;
};






Sql::Sql(const QString &host, const QString &schema, const QString &table, const QString &color, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Sql)
{
    sql_host = host;
    sql_schema = schema;
    sql_table = table;
    sql_color = color;

    ui->setupUi(this);

    dbMysqlLocal = dbMysql;

    setInterfaceSize(0);

    this->setWindowTitle(schema+" • "+table);
    QString style = "QTextEdit {background-color: " + getRgbFromColorName(sql_color) + "}";
    ui->textQuery->setStyleSheet(style);

    if (currentTheme=="light")
        ui->textQuery->setTextColor(QColor("black"));
    else
        ui->textQuery->setTextColor(QColor("white"));

    new SqlHighlighter(ui->textQuery->document());

    QString param = sql_host + ":" + sql_schema + ":" + sql_table;
    QString queryStr = getStringPreference(param);
    if (queryStr == "")
    {
        queryStr = "SELECT * FROM "+table+" LIMIT " + QString::number(pref_sql_limit)+ ";";
    }
    ui->textQuery->setText(queryStr);

    // Espaçador invisível que "empurra" o conteúdo para a esquerda
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QLabel *label = new QLabel("Every ", this);
    QLabel *labelSeconds = new QLabel("second(s) ", this);
    QLabel *labelTimes = new QLabel(" time(s)", this);

    edit = new QLineEdit(this);
    edit->setText("1");
    edit->setFixedWidth(50);
    edit->setPlaceholderText("Seconds");
    QIntValidator *validator = new QIntValidator(0, 3600, this);
    edit->setValidator(validator);

    editTimes = new QLineEdit(this);
    editTimes->setText("5");
    editTimes->setFixedWidth(50);
    editTimes->setPlaceholderText("Times");
    QIntValidator *validatorTimes = new QIntValidator(0, 99999, this);
    editTimes->setValidator(validatorTimes);

    button = new QPushButton(this);
    QIcon icone(":/icons/resources/clock.svg");
    button->setIcon(icone);
    button->setIconSize(QSize(16, 16));
    button->setCheckable(true);
    button->setChecked(false);

    connect(button, &QPushButton::clicked, this, &Sql::on_button_clicked);

    // Temporizador
    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &Sql::on_timer_tick);


    ui->toolBarQuery->addWidget(spacer);
    ui->toolBarQuery->addWidget(label);
    ui->toolBarQuery->addWidget(edit);
    ui->toolBarQuery->addWidget(labelSeconds);
    ui->toolBarQuery->addWidget(editTimes);
    ui->toolBarQuery->addWidget(labelTimes);
    ui->toolBarQuery->addWidget(button);

    on_actionRun_triggered();

    // Menu de contexto
    ui->tableData->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableData, &QTableView::customContextMenuRequested,
            this, &Sql::show_context_menu);

}

Sql::~Sql()
{
    delete ui;
}

void Sql::setInterfaceSize(int increase)
{
    if (increase > 0 && pref_sql_font_size < 30)
    {
        pref_sql_font_size++;
        pref_table_font_size++;
        pref_table_row_height+=2;
    }
    if (increase < 0 && pref_sql_font_size > 6)
    {
        pref_sql_font_size--;
        pref_table_font_size--;
        pref_table_row_height-=2;
    }
    QFont fonte;
    fonte.setFamilies(QStringList()
                           << "Segoe UI"        // Windows
                           << ".SF NS Text"     // macOS (nome interno do San Francisco)
                           << "Ubuntu"          // Ubuntu
                           << "Cantarell"       // GNOME padrão
                           << "Noto Sans"       // fallback moderno do Google
                           << "Sans Serif");    // fallback genérico
    fonte.setPointSize(pref_table_font_size);
    // fonte.setStyleHint(QFont::Monospace);  // opcional, força alinhamento fixo
    ui->tableData->setFont(fonte);
    // ui->tableData->setFixedHeight(pref_table_row_height);

    QFont fonteQuery;
    fonteQuery.setFamilies(QStringList() << "Consolas" << "Menlo" << "Courier New" << "Monospace");
    fonteQuery.setPointSize(pref_sql_font_size);
    fonteQuery.setStyleHint(QFont::Monospace);
    ui->textQuery->setFont(fonteQuery);

    ui->tableData->viewport()->update();
    ui->textQuery->viewport()->update();
}

void Sql::refresh_structure()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    if (dbMysqlLocal.open())
    {

        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + sql_host));
        QString queryStr = QString("USE %1;").arg(sql_schema);
        if (query.exec(queryStr)) {
            QString queryStr = "DESCRIBE " + sql_table;
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

                // ui->tableView->setModel(model);
                // ui->tableView->resizeColumnsToContents();
                // ui->tableView->horizontalHeader()->setStretchLastSection(true);

            } else {
                qCritical() << "Erro ao obter estrutura:" << query.lastError().text();
            }
            // Aplicar delegates dinamicamente após atualizar os dados
            // int cols = ui->tableView->model()->columnCount();
            // for (int col = 0; col < cols; ++col) {
            //     QString header = ui->tableView->model()->headerData(col, Qt::Horizontal).toString().toLower();
            //     if (header == "field") {
            //         ui->tableView->setItemDelegateForColumn(col, new RegexDelegateName(this));
            //     } else if (header == "type") {
            //         ui->tableView->setItemDelegateForColumn(col, new RegexDelegateType(this));
            //     } else if (header == "null") {
            //         ui->tableView->setItemDelegateForColumn(col, new RegexDelegateYesNo(this));
            //     }
            // }
            // connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentChanged,
            //         this, [this](const QModelIndex &current, const QModelIndex &) {
            //             if (current.isValid()) {
            //                 editIndex = current;
            //                 previousValue = ui->tableView->model()->data(current);
            //             }
            //         });

            // connect(ui->tableView->model(), &QAbstractItemModel::dataChanged,
            //         this, &Structure::on_tableData_changed);


            // // Seleciona a primeira linha após carregar os dados
            // if (ui->tableView->model()->rowCount() > 0) {
            //     QModelIndex firstIndex = ui->tableView->model()->index(0, 0);
            //     ui->tableView->selectionModel()->select(firstIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            //     ui->tableView->setCurrentIndex(firstIndex);
            // }
        }
    } else {
        qCritical() << "Failed to connect to the database";
    }

    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
}


void Sql::formatSqlText()
{
    QString texto = ui->textQuery->toPlainText();

    // Lista de palavras que devem iniciar nova linha
    QStringList palavrasChave = {
        "FROM", "LEFT JOIN", "INNER JOIN", "RIGHT JOIN",
        "WHERE", "ORDER", "GROUP BY", "LIMIT"
    };

    // Substitui cada ocorrência com \n + palavra
    for (const QString &palavra : palavrasChave) {
        QRegularExpression re(QStringLiteral("\\b%1\\b").arg(QRegularExpression::escape(palavra)), QRegularExpression::CaseInsensitiveOption);
        texto.replace(re, "\n" + palavra);
    }

    // Remove espaços duplos e ajusta visual
    texto = texto.trimmed().replace(QRegularExpression("[ \\t]+"), " ");

    ui->textQuery->setPlainText(texto);
}


void Sql::query2TableView(QTableView *tableView, const QString &queryStr, const QString &comando)
{
    QStandardItemModel *model = new QStandardItemModel(tableView);

    QSqlQuery query(dbMysqlLocal);
    if (!query.exec(queryStr)) {
        qWarning() << "Erro na query:" << query.lastError().text();
        delete model;
        return;
    }

    currentRecord = query.record();
    int colCount = currentRecord.count();
    idPosition = -1;
    hasId = false;

    for (int i = 0; i < colCount; ++i) {
        model->setHorizontalHeaderItem(i, new QStandardItem(currentRecord.fieldName(i)));
        if (currentRecord.fieldName(i) == "id") {
            hasId = true;
            idPosition = i;
        }
    }

    while (query.next()) {
        QList<QStandardItem*> rowItems;
        for (int col = 0; col < colCount; ++col) {
            QVariant value = query.value(col);
            QVariant::Type type = currentRecord.field(col).type();

            QString displayText;

            switch (type) {
            case QVariant::Date:
                displayText = value.toDate().toString("yyyy-MM-dd 00:00:00.000");
                break;
            case QVariant::Time:
                displayText = value.toTime().toString("1970-01-01 HH:mm:ss.zzz");
                break;
            case QVariant::DateTime:
                displayText = value.toDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
                break;
            default:
                displayText = value.toString();
            }

            QStandardItem *item = new QStandardItem(displayText);
            item->setEditable(true);
            item->setData(displayText, Qt::UserRole);  // Armazena valor original formatado

            // Alinhamento
            switch (type) {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Double:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                break;
            case QVariant::Date:
            case QVariant::Time:
            case QVariant::DateTime:
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                break;
            default:
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                break;
            }

            rowItems << item;
        }
        model->appendRow(rowItems);
    }


    tableView->setModel(model);
    tableView->resizeColumnsToContents();

    if (comando == "SELECT" && hasId && !hasJoin && !hasSubquery) {
        // qDebug() << "- Edição: HABILITADA";
        editEnabled = true;
        tableView->setEditTriggers(
            QAbstractItemView::SelectedClicked |
            QAbstractItemView::EditKeyPressed |
            QAbstractItemView::AnyKeyPressed |
            QAbstractItemView::DoubleClicked
            );


        CustomDelegate *delegate = new CustomDelegate(tableView, currentRecord);
        tableView->setItemDelegate(delegate);

        // Conecta salvamento ao sair do editor
        connect(delegate, &QAbstractItemDelegate::commitData, this, [=](QWidget *editor) {
            QModelIndex index = tableView->currentIndex();
            if (!index.isValid() || !hasId || idPosition < 0)
                return;

            QString fieldName = currentRecord.fieldName(index.column());
            QString idValue = model->item(index.row(), idPosition)->text();
            QString newValue = index.model()->data(index, Qt::EditRole).toString();
            QString oldValue = model->item(index.row(), index.column())->data(Qt::UserRole).toString();

            if (!on_tableData_edit_trigger(idValue, fieldName, newValue)) {
                model->blockSignals(true);  // evita loop de sinais
                model->setData(index, oldValue, Qt::EditRole);
                model->blockSignals(false);

                statusBar()->showMessage(
                    QString("Alteração REJEITADA em '%1'. Valor restaurado: %2")
                    .arg(fieldName, oldValue)
                );
                qWarning() << "Alteração rejeitada. Valor restaurado.";
            } else {
                model->setData(index, newValue, Qt::UserRole); // atualiza valor original
                statusBar()->showMessage(QString("Alteração salva em '%1'").arg(fieldName));
            }
        });



        // Conecta mudança de célula para exibir tipo e tamanho na barra de status
        connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex &current, const QModelIndex &) {
                    if (!currentRecord.isEmpty() && current.isValid()) {
                        QSqlField field = currentRecord.field(current.column());
                        QVariant::Type type = field.type();
                        int fieldSize = field.length();

                        static const QMap<QVariant::Type, QString> sqlTypeMap = {
                            {QVariant::Int, "int"},
                            {QVariant::UInt, "unsigned int"},
                            {QVariant::LongLong, "bigint"},
                            {QVariant::ULongLong, "unsigned bigint"},
                            {QVariant::Double, "double"},
                            {QVariant::Bool, "boolean"},
                            {QVariant::Date, "date"},
                            {QVariant::Time, "time"},
                            {QVariant::DateTime, "datetime"},
                            {QVariant::String, "varchar"},
                            {QVariant::ByteArray, "blob"},
                            {QVariant::Char, "char"},
                            {QVariant::Invalid, "unknown"}
                        };

                        QString sqlTypeName = sqlTypeMap.value(type, QMetaType(type).name());
                        QString info;

                        if (fieldSize == 262140 && sqlTypeName == "varchar") {
                            info = QString("%1 • text").arg(field.name());
                        } else {
                            info = QString("%1 • %2(%3)").arg(field.name(), sqlTypeName).arg(fieldSize);
                        }

                        statusBar()->showMessage(info);
                    }
                });
    } else {
        // qDebug() << "- Edição: DESABILITADA";
        editEnabled = false;
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}



bool Sql::on_tableData_edit_trigger(QString &id, QString &fieldName, QString &newValue)
{
    if (fieldName=="id")
    {
        return false;
    }
    // Remove o caractere apóstrofo ' pra evitar SQLInjection
    newValue.remove('\'');

    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + sql_host);
    QSqlQuery query(dbMysqlLocal);

    QString queryStr = "UPDATE " + sql_table + " SET " + fieldName + " = '" + newValue + "' WHERE id = " + id;
    // qDebug() << queryStr;
    if (!query.exec(queryStr) || query.numRowsAffected() == 0) {
        qWarning() << "Erro na query:" << query.lastError().text();
        return false;
    }
    return true;
}


void Sql::on_actionRun_triggered()
{

    // qDebug() << "host: " << sql_host << " schema: " << sql_schema << " table: " << sql_table;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    // executa a query
    // QSqlQueryModel *model = new QSqlQueryModel(this);
    QString queryStr = ui->textQuery->textCursor().selectedText();
    if (queryStr == "")
    {
        // queryStr = ui->textQuery->toPlainText();

        QTextCursor cursor = ui->textQuery->textCursor();  // textEdit é um ponteiro para QTextEdit
        int cursorPos = cursor.position();
        queryStr = extractCurrentQuery(ui->textQuery->toPlainText(), cursorPos);
        // qDebug() << "Query sob o cursor:" << queryStr;
    }
    if (queryStr != "")
    {
        queryStr = queryStr.trimmed();
        editEnabled = false;

        QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + sql_host);

        QString comando = queryStr.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).value(0).toUpper();


        if (comando == "SELECT")
        {

            // Extrai campos

            // Palavras reservadas mais comuns do MySQL
            QStringList sqlKeywords = {
                "SELECT", "FROM", "WHERE", "JOIN", "LEFT", "RIGHT", "INNER", "OUTER", "ON",
                "GROUP", "ORDER", "BY", "LIMIT", "HAVING", "AS", "AND", "OR", "NOT", "IN", "IS",
                "NULL", "DESC", "ASC", "UNION", "ALL", "DISTINCT", "INSERT", "UPDATE", "DELETE"
            };

            // Extrair tabela principal e alias, evitando palavras reservadas
            QRegularExpression reFrom(R"(FROM\s+(\w+)(?:\s+(\w+))?)", QRegularExpression::CaseInsensitiveOption);

            if (auto match = reFrom.match(queryStr); match.hasMatch()) {
                tableName = match.captured(1).trimmed();
                tableAlias = match.captured(2).trimmed();

                // Valida se o alias não é palavra reservada
                if (!tableAlias.isEmpty() && sqlKeywords.contains(tableAlias.toUpper())) {
                    tableAlias.clear();
                }
            }

            // Extrair SELECT

            QRegularExpression reSelect(R"(SELECT\s+(.*?)\s+FROM)", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
            if (auto match = reSelect.match(queryStr); match.hasMatch()) {
                QStringList rawFields = match.captured(1).split(',', Qt::SkipEmptyParts);
                selectFields = extractFieldsWithPrefix(rawFields, tableName, tableAlias);
            }

            // Extrair WHERE

            QRegularExpression reWhere(R"(WHERE\s+(.*?)(ORDER\s+BY|LIMIT|$))", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
            if (auto match = reWhere.match(queryStr); match.hasMatch()) {
                QString whereClause = match.captured(1).trimmed();
                QRegularExpression reField(R"((\b\w+\.\w+|\b\w+)\s*(=|<>|!=|<|>|<=|>=|LIKE|IS|IN))", QRegularExpression::CaseInsensitiveOption);
                QStringList rawWhereFields;
                auto it = reField.globalMatch(whereClause);
                while (it.hasNext())
                    rawWhereFields << it.next().captured(1).trimmed();
                whereFields = extractFieldsWithPrefix(rawWhereFields, tableName, tableAlias);
            }

            // Extrair ORDER BY
            QStringList orderByFields;
            QRegularExpression reOrder(R"(ORDER\s+BY\s+(.*?)(LIMIT|$))", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption);
            if (auto match = reOrder.match(queryStr); match.hasMatch()) {
                QStringList rawOrderFields = match.captured(1).split(',', Qt::SkipEmptyParts);
                for (QString &f : rawOrderFields) f = f.trimmed();
                orderByFields = extractFieldsWithPrefix(rawOrderFields, tableName, tableAlias);
            }

            // Detectar JOINs
            hasJoin = queryStr.contains(QRegularExpression(R"(\bJOIN\b)", QRegularExpression::CaseInsensitiveOption));

            // Detectar subqueries (SELECT dentro de parênteses)
            hasSubquery = queryStr.contains(QRegularExpression(R"(\(\s*SELECT\s+)", QRegularExpression::DotMatchesEverythingOption | QRegularExpression::CaseInsensitiveOption));

            // ▶ Impressão para teste
            // qDebug() << "- Possui JOIN? " << (hasJoin ? "Sim" : "Não");
            // qDebug() << "- Possui subquery (SELECT aninhado)? " << (hasSubquery ? "Sim" : "Não");

            // ▶ Impressão para teste
            // qDebug() << "- Tabela principal:" << tableName;
            // qDebug() << "- Alias da tabela:" << tableAlias;
            // qDebug() << "- Campos SELECT da tabela principal:" << selectFields;
            // qDebug() << "- Campos WHERE da tabela principal:" << whereFields;
            // qDebug() << "- Campos ORDER BY da tabela principal:" << orderByFields;

        }


        if (comando == "SELECT" || comando == "SHOW" || comando == "DESCRIBE" || comando == "EXPLAIN") {
            // comandos que retornam resultados

            query2TableView(ui->tableData, queryStr, comando);

        } else {
            editEnabled = false;
            ui->tableData->setEditTriggers(QAbstractItemView::NoEditTriggers);

            // comandos como INSERT, UPDATE, DELETE, etc.
            QSqlQuery query(db);
            if (!query.exec(queryStr)) {
                statusMessage("Command error: " + query.lastError().text());
            } else {
                int linhasAfetadas = query.numRowsAffected();
                statusMessage("Success! Line affected: " + QString::number(linhasAfetadas));
            }

            // Limpa a tabela visual, pois não há dados a mostrar
            ui->tableData->setModel(nullptr);
        }

    }
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
}



void Sql::on_actionFormat_triggered()
{
    formatSqlText();
}


void Sql::on_actionIncrease_triggered()
{
    setInterfaceSize(1);
}


void Sql::on_actionReduce_triggered()
{
    setInterfaceSize(-1);
}


void Sql::on_actionSave_triggered()
{
    updatePreferences();
    QString param = sql_host + ":" + sql_schema + ":" + sql_table;
    QString queryStr = ui->textQuery->textCursor().selectedText();
    if (queryStr == "")
    {
        queryStr = ui->textQuery->toPlainText();
    }
    setStringPreference(param, queryStr);
}

void Sql::statusMessage(QString msg)
{
    ui->statusbar->showMessage(QTime::currentTime().toString("HH:mm:ss") + " • " + msg);
}

// Em MainWindow.cpp
void Sql::on_timer_tick()
{
    if (button->isChecked())
    {
        int remaining = editTimes->text().toInt();

        if (remaining <= 1)
        {
            timer->stop();
            editTimes->setText("0");  // opcional, para refletir que chegou a zero
            edit->setEnabled(true);
            button->setChecked(false);
        }
        else
        {
            on_actionRun_triggered();
            if (ui->textQuery->styleSheet() == "QTextEdit {background-color: " + getRgbFromColorName(sql_color) + "}")
            {
                QString style = "QTextEdit {background-color: white}";
                ui->textQuery->setStyleSheet(style);
            } else {
                QString style = "QTextEdit {background-color: " + getRgbFromColorName(sql_color) + "}";
                ui->textQuery->setStyleSheet(style);
            }

            remaining--;
            editTimes->setText(QString::number(remaining));
        }
    }
}

void Sql::on_button_clicked()
{
    if (button->isChecked())
    {
        timer->start(edit->text().toInt()*1000);
        edit->setEnabled(false);
        ui->textQuery->setEnabled(false);
    } else {
        if (timer->isActive())
        {
            edit->setEnabled(true);
            timer->stop();
            ui->textQuery->setEnabled(true);
        }
    }
}





// Menu de contexto

void Sql::on_tableClone_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    int row = selection.first().row();
    QSqlRecord record;
    QStringList fields, values;

    for (int col = 0; col < ui->tableData->model()->columnCount(); ++col) {
        if (ui->tableData->model()->headerData(col, Qt::Horizontal).toString().toLower() == "id")
            continue; // não clonar o campo ID
        QString field = ui->tableData->model()->headerData(col, Qt::Horizontal).toString();
        QString value = ui->tableData->model()->data(ui->tableData->model()->index(row, col)).toString();
        fields << "`" + field + "`";
        values << "'" + value.replace("'", "\'") + "'";
    }

    QString insert = QString("INSERT INTO %1 (%2) VALUES (%3)")
                         .arg(sql_table)
                         .arg(fields.join(", "))
                         .arg(values.join(", "));

    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + sql_host));
    if (!query.exec(insert)) {
        qWarning() << "Erro ao clonar linha:" << query.lastError().text();
    } else {
        refresh_structure();
    }
}

void Sql::on_tableDelete_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    QSet<int> rows;
    for (const QModelIndex &index : selection)
        rows.insert(index.row());

    if (QMessageBox::question(this, "Confirmar exclusão",
                              QString("Deseja excluir %1 linha(s)?").arg(rows.size())) != QMessageBox::Yes)
        return;

    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + sql_host));
    QList<int> sortedRows = rows.values();
    std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

    for (int row : sortedRows) {
        QString id = ui->tableData->model()->data(ui->tableData->model()->index(row, idPosition)).toString();
        QString del = QString("DELETE FROM %1 WHERE id = %2").arg(sql_table).arg(id);
        // qDebug() << del;
        if (!query.exec(del)) {
            qWarning() << "Erro ao excluir linha id=" << id << ":" << query.lastError().text();
        } else {
            static_cast<QStandardItemModel *>(ui->tableData->model())->removeRow(row);
        }
    }
}


void Sql::on_tableCopyInsert_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    QAbstractItemModel *model = ui->tableData->model();

    // Agrupa os valores por linha
    QMap<int, QMap<int, QString>> rowValues;
    for (const QModelIndex &index : selection) {
        int row = index.row();
        int col = index.column();
        QString header = model->headerData(col, Qt::Horizontal).toString();

        // if (header.toLower() == "id") continue;

        QString value = index.data().toString().replace("'", "\\'");
        rowValues[row][col] = "'" + value + "'";
    }

    QStringList insertCommands;
    for (auto it = rowValues.constBegin(); it != rowValues.constEnd(); ++it) {
        const QMap<int, QString> &colMap = it.value();

        QStringList columns;
        QStringList values;
        for (auto colIt = colMap.constBegin(); colIt != colMap.constEnd(); ++colIt) {
            QString columnName = model->headerData(colIt.key(), Qt::Horizontal).toString();
            columns << columnName;
            values << colIt.value();
        }

        QString query = QString("INSERT INTO %1 (%2) VALUES (%3);")
                            .arg(sql_table)
                            .arg(columns.join(", "))
                            .arg(values.join(", "));
        insertCommands << query;
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(insertCommands.join("\n"));
}


void Sql::on_tableCopyUpdate_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    QAbstractItemModel *model = ui->tableData->model();

    QMap<int, QMap<int, QString>> rowValues;

    int idColumn = -1;

    // Identifica a coluna "Id"
    for (int col = 0; col < model->columnCount(); ++col) {
        QString header = model->headerData(col, Qt::Horizontal).toString();
        if (header.toLower() == "id") {
            idColumn = col;
            break;
        }
    }

    if (idColumn == -1) {
        qWarning() << "Coluna 'Id' não encontrada.";
        return;
    }

    // Coleta os dados selecionados (exceto Id)
    for (const QModelIndex &index : selection) {
        int row = index.row();
        int col = index.column();

        if (col == idColumn) continue;

        QString value = index.data().toString().replace("'", "\\'");
        rowValues[row][col] = "'" + value + "'";
    }

    QStringList updateCommands;

    for (auto it = rowValues.constBegin(); it != rowValues.constEnd(); ++it) {
        int row = it.key();
        const QMap<int, QString> &colMap = it.value();

        // Recupera o valor da coluna Id diretamente do modelo
        QModelIndex idIndex = model->index(row, idColumn);
        QString idValue = idIndex.data().toString();

        if (idValue.isEmpty()) {
            qWarning() << "Linha" << row << "não possui valor de ID.";
            continue;
        }

        QStringList assignments;
        for (auto colIt = colMap.constBegin(); colIt != colMap.constEnd(); ++colIt) {
            QString columnName = model->headerData(colIt.key(), Qt::Horizontal).toString();
            assignments << QString("%1 = %2").arg(columnName, colIt.value());
        }

        QString query = QString("UPDATE %1 SET %2 WHERE Id = %3;")
                            .arg(sql_table)
                            .arg(assignments.join(", "))
                            .arg(idValue);
        updateCommands << query;
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(updateCommands.join("\n"));
}

void Sql::on_tableCopyCsv_triggered()
{
    QModelIndexList selection = ui->tableData->selectionModel()->selectedIndexes();
    if (selection.isEmpty()) return;

    std::sort(selection.begin(), selection.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() == b.row() ? a.column() < b.column() : a.row() < b.row();
    });

    QAbstractItemModel *model = ui->tableData->model();

    // Obter colunas únicas selecionadas (em ordem)
    QSet<int> selectedColumnsSet;
    for (const QModelIndex &index : selection)
        selectedColumnsSet.insert(index.column());

    QList<int> selectedColumns = selectedColumnsSet.values();
    std::sort(selectedColumns.begin(), selectedColumns.end());

    // Cabeçalhos
    QStringList headerRow;
    for (int column : selectedColumns) {
        QString header = model->headerData(column, Qt::Horizontal).toString();
        header.replace("\"", "\"\"");
        headerRow << "\"" + header + "\"";
    }

    QStringList csvText;
    csvText << headerRow.join(";");

    int currentRow = selection.first().row();
    QStringList csvRow;
    csvRow.fill("", selectedColumns.size());

    for (const QModelIndex &index : selection) {
        if (index.row() != currentRow) {
            csvText << csvRow.join(";");
            csvRow.fill("", selectedColumns.size());
            currentRow = index.row();
        }

        int colIndex = selectedColumns.indexOf(index.column());
        if (colIndex != -1) {
            QVariant data = index.data();
            QString str = data.toString();
            str.replace("\"", "\"\"");

            // Detectar o tipo real do valor
            QVariant::Type type = data.type();

            bool isNumeric = (type == QVariant::Int ||
                              type == QVariant::UInt ||
                              type == QVariant::LongLong ||
                              type == QVariant::ULongLong ||
                              type == QVariant::Double);

            bool needsQuotes = !isNumeric ||
                               str.contains(";") ||
                               str.contains("\n") ||
                               str.contains("\"") ||
                               str.startsWith("{") || str.startsWith("[");  // possível JSON

            csvRow[colIndex] = needsQuotes ? "\"" + str + "\"" : str;
        }
    }

    csvText << csvRow.join(";");

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(csvText.join("\n"));
}


void Sql::on_tableCRUDGfw_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText("<?php \n ?>");
}

void Sql::on_tableCRUDLaravel_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText("<?php \n ?>");
}

void Sql::show_context_menu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *tableClone = menu.addAction("Clone row");
    QAction *tableDelete = menu.addAction("Delete row(s)");
    menu.addSeparator();
    QAction *tableCopyInsert = menu.addAction("Copy as INSERT");
    QAction *tableCopyUpdate = menu.addAction("Copy as UPDATE");
    QAction *tableCopyCsv = menu.addAction("Copy as CSV");
    menu.addSeparator();
    QAction *tableCRUDGfw= menu.addAction("CRUD gFW");
    QAction *tableCRUDLaravel = menu.addAction("CRUD Laravel");
    menu.addSeparator();
    QAction *tableExportCSV = menu.addAction("Export as CSV");
    QAction *tableExportPDF = menu.addAction("Export as PDF");

    QAction *selectedAction = menu.exec(ui->tableData->viewport()->mapToGlobal(pos));

    if (selectedAction == tableClone) {
        on_tableClone_triggered();
    }
    else if (selectedAction == tableDelete) {
        on_tableDelete_triggered();
    }
    else if (selectedAction == tableCopyInsert) {
        on_tableCopyInsert_triggered();
    }
    else if (selectedAction == tableCopyUpdate) {
        on_tableCopyUpdate_triggered();
    }
    else if (selectedAction == tableCopyCsv) {
        on_tableCopyCsv_triggered();
    }
    else if (selectedAction == tableCRUDGfw) {
        on_tableCRUDGfw_triggered();
    }
    else if (selectedAction == tableCRUDLaravel) {
        on_tableCRUDLaravel_triggered();
    }
}

