#include "structure.h"
#include "ui_structure.h"

#include <QDebug>
#include <QLabel>
#include <QLineEdit>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <functions.h>

QString str_host = "";
QString str_schema = "";
QString str_table = "";

Structure::Structure(QString &host, QString &schema, QString &table, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Structure)
{
    ui->setupUi(this);

    str_host = host;
    str_schema = schema;
    str_table = table;

    this->setWindowTitle(schema + " • " + table);
    refresh_structure();
}

Structure::~Structure()
{
    delete ui;
}

void Structure::refresh_structure()
{
    QString consulta = "DESCRIBE " + str_table;
    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + str_host));

    if (query.exec(consulta)) {
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
}
