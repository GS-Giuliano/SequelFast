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

Sql::Sql(const QString &host, const QString &schema, const QString &table, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Sql)
{
    sql_host = host;
    sql_schema = schema;
    sql_table = table;

    ui->setupUi(this);

    this->setWindowTitle(sql_schema+" • "+table);
    new SqlHighlighter(ui->textQuery->document());
    ui->textQuery->setText("SELECT * FROM "+table+" LIMIT 1000;");

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
    QString consulta = ui->textQuery->toPlainText();
    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + sql_host);
    model->setQuery(consulta, db);

    if (model->lastError().isValid()) {
        ui->statusbar->showMessage("Query error: " + model->lastError().text());
        delete model;  // opcional, já tem pai, mas se reutilizar...
        return;
    }
    ui->tableData->setSortingEnabled(true);
    ui->tableData->resizeColumnsToContents();
    ui->tableData->setModel(model);  // exibe os resultados na tabela
    ui->statusbar->showMessage("Query executed successfully!");
}


void Sql::on_actionFormat_triggered()
{
    formatSqlText();
}

