#include "connection.h"
#include "ui_connection.h"
#include "functions.h"

extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;
extern QJsonArray connections;
extern QString actual_host;
extern QString actual_schema;
extern QString actual_table;
extern QJsonArray colors;

QString thatHost;
QString thatColor = "white";

#include <QJsonArray>
#include <QJsonObject>
#include "ui_connection.h"

Connection::Connection(QString selectedHost, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Connection)
{
    ui->setupUi(this);
    ui->lineName->setText(selectedHost);

    thatHost = selectedHost;
    QJsonObject item = getConnection(thatHost);

    ui->lineHost->setText(item["host"].toString());
    ui->linePort->setText(item["port"].toString());
    ui->lineUser->setText(item["user"].toString());
    ui->linePass->setText(item["pass"].toString());
    ui->lineSchema->setText(item["schema"].toString());

    ui->lineSSHhost->setText(item["ssh_host"].toString());
    ui->lineSSHport->setText(item["ssh_port"].toString());
    ui->lineSSHuser->setText(item["ssh_user"].toString());
    ui->lineSSHpass->setText(item["ssh_pass"].toString());
    ui->lineSSHkey->setText(item["ssh_keyfile"].toString());
    if (item["shared"].toString() == "1")
    {
        ui->sharedFavorites->setChecked(true);
    }

    thatColor = item["color"].toString();
    int color_number = 0;

    // Busca a posição da cor no colorMap
    for (int i = 0; i < colors.size(); ++i) {
        QJsonObject obj = colors[i].toObject();
        if (obj["name"].toString() == thatColor) {
            color_number = i;
            break;
        }
    }

    ui->dial->setValue(color_number);
}


// Connection::Connection(QString selectedHost, QWidget *parent)
//     : QDialog(parent)
//     , ui(new Ui::Connection)
// {

//     ui->setupUi(this);
//     ui->lineName->setText(selectedHost);


//     thatHost = selectedHost;
//     QJsonObject item = getConnection(thatHost);
//     ui->lineHost->setText(item["host"].toVariant().toString());
//     ui->linePort->setText(item["port"].toVariant().toString());
//     ui->lineUser->setText(item["user"].toVariant().toString());
//     ui->linePass->setText(item["pass"].toVariant().toString());
//     ui->lineSchema->setText(item["schema"].toVariant().toString());

//     ui->lineSSHhost->setText(item["ssh_host"].toVariant().toString());
//     ui->lineSSHport->setText(item["ssh_port"].toVariant().toString());
//     ui->lineSSHuser->setText(item["ssh_user"].toVariant().toString());
//     ui->lineSSHpass->setText(item["ssh_pass"].toVariant().toString());
//     ui->lineSSHkey->setText(item["ssh_keyfile"].toVariant().toString());

//     thatColor = item["color"].toVariant().toString();
//     int color_number = 0;

//     if (thatColor == "white") {color_number = 0; };
//     if (thatColor == "brown") {color_number = 1; };
//     if (thatColor == "red")   {color_number = 2; };
//     if (thatColor == "purple"){color_number = 3; };
//     if (thatColor == "blue")  {color_number = 4; };
//     if (thatColor == "green") {color_number = 5; };
//     if (thatColor == "yellow"){color_number = 6; };
//     if (thatColor == "orange"){color_number = 7; };
//     if (thatColor == "grey")  {color_number = 8; };

//     ui->dial->setValue(color_number);
// }

Connection::~Connection()
{
    delete ui;
}

void Connection::saveConnection()
{
    if (!dbPreferences.open()) {
        qCritical() << "Erro ao abrir o banco de dados SQLite:" << dbPreferences.lastError().text();
    }

    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    QString updateSql = "UPDATE conns SET shared = :shared, name = :new_name, schema = :new_schema, color = :new_color, host = :new_host, user = :new_user, pass = :new_pass, port = :new_port, ssh_host = :new_sshhost, ssh_user = :new_sshuser, ssh_pass = :new_sshpass, ssh_port = :new_sshport, ssh_keyfile = :new_sshkey WHERE name = :name_to_update";
    query.prepare(updateSql);
    query.bindValue(":new_color", thatColor);
    query.bindValue(":shared", ui->sharedFavorites->isChecked() ? 1 : 0);
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
        qWarning() << "Erro ao salvar host:" << query.lastError().text();
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

    dbMysql = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection_" + ui->lineName->text());

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
    }
    else {
        qDebug() << "Conexão bem-sucedida!";
        dbMysql.close();
    }
}




void Connection::on_dial_valueChanged(int value)
{
    if (value >= 0 && value < colors.size()) {
        QJsonObject obj = colors[value].toObject();
        QString colorName = obj["name"].toString();
        QString colorRgb = obj["rgb"].toString();

        ui->dial->setStyleSheet(QString("QDial {background-color: %1}").arg(colorRgb));
        thatColor = colorName;
        // qDebug() << "Cor:" << thatColor << "RGB:" << colorRgb;
    }

    // if (value == 0) {ui->dial->setStyleSheet("QDial {background-color: white}"); thatColor="white";};
    // if (value == 1) {ui->dial->setStyleSheet("QDial {background-color: #EDBB99}"); thatColor="brown";};
    // if (value == 2) {ui->dial->setStyleSheet("QDial {background-color: #FFBAAB}"); thatColor="red";};
    // if (value == 3) {ui->dial->setStyleSheet("QDial {background-color: #DBBAFF}"); thatColor="purple";};
    // if (value == 4) {ui->dial->setStyleSheet("QDial {background-color: #ABD8FF}"); thatColor="blue";};
    // if (value == 5) {ui->dial->setStyleSheet("QDial {background-color: #ABEBC6}"); thatColor="green";};
    // if (value == 6) {ui->dial->setStyleSheet("QDial {background-color: #FCF3CF}"); thatColor="yellow";};
    // if (value == 7) {ui->dial->setStyleSheet("QDial {background-color: #FFE6BA}"); thatColor="orange";};
    // if (value == 8) {ui->dial->setStyleSheet("QDial {background-color: #D7DBDD}"); thatColor="grey";};
    // qDebug() << "Cor: " << thatColor;
}

