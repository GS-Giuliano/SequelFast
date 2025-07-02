#include "connection.h"
#include "ui_connection.h"
#include <functions.h>

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;

extern QJsonArray connections;

extern QString actual_host;
extern QString actual_schema;
extern QString actual_table;

QString thatHost;

Connection::Connection(QString selectedHost, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Connection)
{

    ui->setupUi(this);
    ui->lineName->setText(selectedHost);


    thatHost = selectedHost;
    QJsonObject item = getConnection(thatHost);
    ui->lineHost->setText(item["host"].toVariant().toString());
    ui->linePort->setText(item["port"].toVariant().toString());
    ui->lineUser->setText(item["user"].toVariant().toString());
    ui->linePass->setText(item["pass"].toVariant().toString());
    ui->lineSchema->setText(item["schema"].toVariant().toString());

    ui->lineSSHhost->setText(item["ssh_host"].toVariant().toString());
    ui->lineSSHport->setText(item["ssh_port"].toVariant().toString());
    ui->lineSSHuser->setText(item["ssh_user"].toVariant().toString());
    ui->lineSSHpass->setText(item["ssh_pass"].toVariant().toString());
    ui->lineSSHkey->setText(item["ssh_keyfile"].toVariant().toString());

    QString color = item["color"].toVariant().toString();
    if (color == "black") {ui->radioButtonBlack->setChecked(true);};
    if (color == "blue") {ui->radioButtonBlue->setChecked(true);};
    if (color == "red") {ui->radioButtonRed->setChecked(true);};
    if (color == "brown") {ui->radioButtonBrown->setChecked(true);};
    if (color == "grey") {ui->radioButtonGrey->setChecked(true);};
    if (color == "orange") {ui->radioButtonOrange->setChecked(true);};
    if (color == "purple") {ui->radioButtonPurple->setChecked(true);};

}

Connection::~Connection()
{
    delete ui;
}

void Connection::saveConnection()
{
    QString color = "";
    if (ui->radioButtonBlack->isChecked()) { color = "black"; }
    if (ui->radioButtonGrey->isChecked()) { color = "grey"; }
    if (ui->radioButtonBrown->isChecked()) { color = "brown"; }
    if (ui->radioButtonGreen->isChecked()) { color = "green"; }
    if (ui->radioButtonBlue->isChecked()) { color = "blue"; }
    if (ui->radioButtonOrange->isChecked()) { color = "orange"; }
    if (ui->radioButtonRed->isChecked()) { color = "red"; }
    if (ui->radioButtonPurple->isChecked()) { color = "purple"; }

    if (!dbPreferences.open()) {
        qCritical() << "Erro ao abrir o banco de dados SQLite:" << dbPreferences.lastError().text();
    }

    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    QString updateSql = "UPDATE conns SET name = :new_name, schema = :new_schema, color = :new_color, host = :new_host, user = :new_user, pass = :new_pass, port = :new_port, ssh_host = :new_sshhost, ssh_user = :new_sshuser, ssh_pass = :new_sshpass, ssh_port = :new_sshport, ssh_keyfile = :new_sshkey WHERE name = :name_to_update";
    query.prepare(updateSql);
    query.bindValue(":new_color", color);
    query.bindValue(":new_name", ui->lineName->text());
    query.bindValue(":new_schema", ui->lineSchema->text());
    query.bindValue(":new_host", ui->lineHost->text());
    query.bindValue(":new_port", ui->linePort->text());
    query.bindValue(":new_user", ui->lineUser->text());
    query.bindValue(":new_pass", ui->linePass->text());
    query.bindValue(":new_sshhost", ui->lineSSHhost->text());
    query.bindValue(":new_sshport", ui->lineSSHport->text());
    query.bindValue(":new_sshuser", ui->lineSSHuser->text());
    query.bindValue(":new_sshpass", ui->lineSSHpass->text());
    query.bindValue(":new_sshkey", ui->lineSSHkey->text());

    query.bindValue(":name_to_update", thatHost);
    if (!query.exec()) {
        qWarning() << "Erro ao testar host:" << query.lastError().text();
    }
    actual_host = ui->lineName->text();
    openPreferences();
}


void Connection::on_buttonCancel_clicked()
{
    reject();
}


void Connection::on_buttonSave_clicked()
{
    saveConnection();
    accept();
}


void Connection::on_buttonRemove_clicked()
{
    deleteConnection(thatHost);
    accept();
}


void Connection::on_buttonConnect_clicked()
{
    saveConnection();
    QJsonObject item = getConnection(ui->lineName->text());

    // qDebug() << "Tentando conexao...";
    // qDebug() << " Host: " << item["host"].toVariant().toString();
    // qDebug() << " User: " << item["user"].toVariant().toString();
    // qDebug() << " Pass: " << item["pass"].toVariant().toString();

    dbMysql = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection_"+ui->lineName->text());

    dbMysql.setHostName(item["host"].toVariant().toString());
    dbMysql.setDatabaseName(item["schema"].toVariant().toString());
    dbMysql.setPort(item["port"].toVariant().toInt());
    dbMysql.setUserName(item["user"].toVariant().toString());
    dbMysql.setPassword(item["pass"].toVariant().toString());

    if (!dbMysql.open()) {
        QMessageBox::information(this,
                                 "Connection",
                                 "Connection failed! Check parameters and try again",
                                 QMessageBox::Ok
                                 );
    } else {
        qDebug() << "Conexão bem-sucedida!";
        dbMysql.close();
    }
}

