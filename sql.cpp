#include "sql.h"
#include "ui_sql.h"
#include "sqlhighlighter.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QTimer>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QToolButton>
#include <QPushButton>

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
QString sql_color;

extern int pref_sql_limit;
extern int pref_table_row_height;
extern int pref_table_font_size;
extern int pref_sql_font_size;

QLineEdit *edit;
QLineEdit *editTimes;
QPushButton *button;
QTimer *timer;


Sql::Sql(const QString &host, const QString &schema, const QString &table, const QString &color, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Sql)
{
    sql_host = host;
    sql_schema = schema;
    sql_table = table;
    sql_color = color;

    ui->setupUi(this);

    setInterfaceSize(0);

    this->setWindowTitle(schema+" • "+table);
    QString style = "QTextEdit {background-color: " + getRgbFromColorName(sql_color) + "}";
    ui->textQuery->setStyleSheet(style);

    new SqlHighlighter(ui->textQuery->document());
    ui->textQuery->setText("SELECT * FROM "+table+" LIMIT " + QString::number(pref_sql_limit)+ ";");

    // Espaçador invisível que "empurra" o conteúdo para a esquerda
    QWidget *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QLabel *label = new QLabel("Every ", this);
    QLabel *labelSeconds = new QLabel("second(s) ", this);
    QLabel *labelTimes = new QLabel(" time(s)", this);

    edit = new QLineEdit(this);
    edit->setText("5");
    edit->setFixedWidth(50);
    QIntValidator *validator = new QIntValidator(0, 3600, this);
    edit->setValidator(validator);

    editTimes = new QLineEdit(this);
    editTimes->setText("5");
    editTimes->setFixedWidth(50);
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
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    // executa a query
    // QSqlQueryModel *model = new QSqlQueryModel(this);
    QString consulta = ui->textQuery->textCursor().selectedText();
    if (consulta == "")
    {
        consulta = ui->textQuery->toPlainText();
    }
    if (consulta != "")
    {
        QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + sql_host);

        QString comando = consulta.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).value(0).toUpper();

        if (comando == "SELECT" || comando == "SHOW" || comando == "DESCRIBE" || comando == "EXPLAIN") {
            // comandos que retornam resultados
            QSqlQueryModel *model = new QSqlQueryModel(this);
            model->setQuery(consulta, db);

            if (model->lastError().isValid()) {
                statusMessage("Query error: " + model->lastError().text());
                delete model;
            } else {
                ui->tableData->setModel(model);
                ui->tableData->resizeColumnsToContents();
                ui->tableData->setSortingEnabled(true);
                statusMessage("Success! Rows: " + QString::number(model->rowCount()));
            }
        } else {
            // comandos como INSERT, UPDATE, DELETE, etc.
            QSqlQuery query(db);
            if (!query.exec(consulta)) {
                statusMessage("Command error: " + query.lastError().text());
            } else {
                int linhasAfetadas = query.numRowsAffected();
                statusMessage("Success! Line affected: " + QString::number(linhasAfetadas));
            }

            // Limpa a tabela visual, pois não há dados a mostrar
            ui->tableData->setModel(nullptr);
        }

        // model->setQuery(consulta, db);

        // if (model->lastError().isValid()) {
        //     statusMessage("Query error: " + model->lastError().text());
        //     delete model;  // opcional, já tem pai, mas se reutilizar...
        //     QApplication::restoreOverrideCursor();
        //     QApplication::processEvents();
        //     return;
        // }
        // ui->tableData->setModel(model);  // exibe os resultados na tabela
        // ui->tableData->resizeColumnsToContents();
        // ui->tableData->setSortingEnabled(true);
        // statusMessage("Query executed successfully!");
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
            qDebug() << "Timer finalizado!";
            timer->stop();
            editTimes->setText("0");  // opcional, para refletir que chegou a zero
            edit->setEnabled(true);
            button->setChecked(false);
        }
        else
        {
            qDebug() << "Timer disparado!";
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
        // Inicia o timer com intervalo de 1000ms (1 segundo)
        qDebug() << "Timer iniciado durante " << edit->text() << " por " << editTimes->text() << " vezes";
        timer->start(edit->text().toInt()*1000);
        edit->setEnabled(false);
    }
}
