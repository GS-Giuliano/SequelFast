#include "sql.h"
#include "ui_sql.h"
#include "sqlhighlighter.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <functions.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;

QString sql_host;
QString sql_schema;
QString sql_table;

extern int pref_sql_limit;
extern int pref_table_row_height;
extern int pref_table_font_size;
extern int pref_sql_font_size;

Sql::Sql(const QString &host, const QString &schema, const QString &table, const QString &color, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Sql)
{
    sql_host = host;
    sql_schema = schema;
    sql_table = table;

    ui->setupUi(this);

    setInterfaceSize(0);

    this->setWindowTitle(sql_schema+" • "+table);

    QString style = "QTextEdit {background-color: " + getRgbFromColorName(color) + "}";

    ui->textQuery->setStyleSheet(style);

    new SqlHighlighter(ui->textQuery->document());
    ui->textQuery->setText("SELECT * FROM "+table+" LIMIT " + QString::number(pref_sql_limit)+ ";");

    QLabel *label = new QLabel("Autorun on every ", this);
    QLabel *labelSeconds = new QLabel("second(s)", this);
    QLineEdit *edit = new QLineEdit(this);
    edit->setText("0");
    edit->setFixedWidth(60);
    QIntValidator *validator = new QIntValidator(0, 3600, this);
    edit->setValidator(validator);

    ui->toolBarData->addWidget(label);
    ui->toolBarData->addWidget(edit);
    ui->toolBarData->addWidget(labelSeconds);

    // Espaçador invisível que "empurra" o conteúdo para a esquerda
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->toolBarData->addWidget(spacer);
    on_actionRun_triggered();
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


void Sql::on_actionRun_triggered()
{
    // executa a query
    QSqlQueryModel *model = new QSqlQueryModel(this);
    QString consulta = ui->textQuery->textCursor().selectedText();
    if (consulta == "")
    {
        consulta = ui->textQuery->toPlainText();
    }
    if (consulta != "")
    {
        QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + sql_host);
        model->setQuery(consulta, db);

        if (model->lastError().isValid()) {
            ui->statusbar->showMessage("Query error: " + model->lastError().text());
            delete model;  // opcional, já tem pai, mas se reutilizar...
            return;
        }

        ui->tableData->setModel(model);  // exibe os resultados na tabela
        ui->tableData->resizeColumnsToContents();
        ui->tableData->setSortingEnabled(true);
        ui->statusbar->showMessage("Query executed successfully!");
    }
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
}

