#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QStringListModel>
#include <QMessageBox>
#include <QStringListModel>
#include <QSortFilterProxyModel>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <functions.h>
#include <connection.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;

extern QString actual_host;
extern QString actual_schema;
extern QString actual_table;

int newConnectionCount = 0;



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    openPreferences();

    ui->setupUi(this);
    refresh_connections();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::host_connect(QString selectedHost)
{
    QJsonObject item = getConnection(selectedHost);

    // qDebug() << "Tentando conexao...";
    // qDebug() << " Host: " << item["host"].toVariant().toString();
    // qDebug() << " User: " << item["user"].toVariant().toString();
    // qDebug() << " Pass: " << item["pass"].toVariant().toString();

    dbMysql = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection_"+selectedHost);

    dbMysql.setHostName(item["host"].toVariant().toString());
    dbMysql.setDatabaseName(item["schema"].toVariant().toString());
    dbMysql.setPort(item["port"].toVariant().toInt());
    dbMysql.setUserName(item["user"].toVariant().toString());
    dbMysql.setPassword(item["pass"].toVariant().toString());

    if (!dbMysql.open()) {
        QMessageBox::information(this,
                                 "Connection failed!",
                                 "Check parameters and try again",
                                 QMessageBox::Ok
                                 );
        return(false);
    } else {
        refresh_schemas(selectedHost);
    }
    return(true);
}


void MainWindow::refresh_connections() {
    ui->toolBoxLeft->setCurrentIndex(2);
    ui->lineEditConns->setText("");
    QStringList lista;
    for (const QJsonValue &valor : connections) {
        if (valor.isObject()) {
            QJsonObject item = valor.toObject();
            lista.append(item["name"].toVariant().toString());
        }
    }
    QStringListModel *modelo = new QStringListModel(this);
    modelo->setStringList(lista);

    // Proxy de filtro
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(modelo);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);  // ignora maiúsculas/minúsculas

    ui->listViewConns->setModel(proxy);

    connect(ui->lineEditConns, &QLineEdit::textChanged, this, [=](const QString &texto) {
        QString pattern = QString("(%1)").arg(texto);
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        proxy->setFilterRegularExpression(re);
    });
}

void MainWindow::refresh_schemas(QString selectedHost)
{
    if (!dbMysql.open()) {
        QMessageBox::information(this,
                                 "Connection failed!",
                                 "Check parameters and try again",
                                 QMessageBox::Ok
                                 );
    } else {
        QJsonObject item = getConnection(selectedHost);
        QSqlQuery query(QSqlDatabase::database("mysql_connection_"+selectedHost));

        // QStringList lista;
        // for (const QJsonValue &valor : connections) {
        //     if (valor.isObject()) {
        //         lista.append(item["name"].toVariant().toString());
        //     }
        // }

        if (query.exec("SHOW DATABASES")) {
            ui->lineEditSchemas->setText("");
            QStringList lista;
            int sel = -1;
            int cnt = 0;
            while (query.next()) {
                // int id = query.value("id").toInt();
                QString name = query.value(0).toString();
                lista.append(name);
                if (item["schema"].toVariant().toString() == name)
                {
                    sel = cnt;
                }
                cnt++;
            }
            QStringListModel *modelo = new QStringListModel(this);
            modelo->setStringList(lista);

            // Proxy de filtro
            QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
            proxy->setSourceModel(modelo);
            proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

            ui->listViewSchemas->setModel(proxy);

            connect(ui->lineEditSchemas, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);
            if (sel > -1)
            {
                QModelIndex index = ui->listViewSchemas->model()->index(sel, 0);
                ui->listViewSchemas->setCurrentIndex(index);
                refresh_tables(selectedHost, item["schema"].toVariant().toString());
            }
        }
    }
}

void MainWindow::refresh_schema(QString selectedSchema)
{
    if (!dbMysql.open()) {
        QMessageBox::information(this,
                                 "Connection failed!",
                                 "Check parameters and try again",
                                 QMessageBox::Ok
                                 );
    } else {
        // Consulta SQL
        QSqlQuery query(QSqlDatabase::database("mysql_connection_"+actual_host));

        if (query.exec("USE "+selectedSchema)) {
            refresh_tables(actual_host, selectedSchema);
            ui->toolBoxLeft->setCurrentIndex(0);
        }
    }
}

void MainWindow::refresh_tables(QString selectedHost, QString selectedSchema) {
    QSqlQuery query(QSqlDatabase::database("mysql_connection_"+selectedHost));
    if (query.exec("SHOW TABLES")) {
        ui->lineEditTables->setText("");
        QStringList lista;
        int sel = -1;
        int cnt = 0;
        while (query.next()) {
            // int id = query.value("id").toInt();
            QString name = query.value(0).toString();
            lista.append(name);
            if (false)
            {
                sel = cnt;
            }
            cnt++;
        }
        QStringListModel *modelo = new QStringListModel(this);
        modelo->setStringList(lista);

        // Proxy de filtro
        QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(modelo);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        // proxy->setFilterRole(Qt::DisplayRole);
        ui->listViewTables->setModel(proxy);

        connect(ui->lineEditTables, &QLineEdit::textChanged, this, [=](const QString &texto) {
            QString pattern = QString("(%1)").arg(texto);
            QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
            proxy->setFilterRegularExpression(re);
        });

        if (sel > -1)
        {
            QModelIndex index = ui->listViewTables->model()->index(sel, 0);
            ui->listViewTables->setCurrentIndex(index);
        }
    } else {
        qDebug() << " Erro ao buscar tabelas";
    }

}


void MainWindow::on_listViewConns_clicked(const QModelIndex &index)
{
    actual_host = index.data(Qt::DisplayRole).toString();

    if (ui->checkBoxConns->isChecked())
    {
        if (index.isValid()) {
            Connection *janela = new Connection(actual_host, this);
            int result = janela->exec();
            if (result == QDialog::Accepted) {
                refresh_connections();
            }
        }
    } else {
        QModelIndex i = ui->listViewConns->currentIndex();
        if (i.isValid()) {
            if (host_connect(actual_host))
            {
                ui->toolBoxLeft->setCurrentIndex(1);
            }
        }
    }
}


void MainWindow::on_listViewSchemas_clicked(const QModelIndex &index)
{
    actual_schema = index.data(Qt::DisplayRole).toString();
    refresh_schema(actual_schema);
}


void MainWindow::on_actionNew_connection_triggered()
{
    QString selected ;
    bool fez = false;
    newConnectionCount = 0;
    while(!fez)
    {
        selected = "New connection";
        if (newConnectionCount > 0) {
            selected+=" " +std::to_string(newConnectionCount);
        }
        newConnectionCount++;
        fez = addConnection(selected);
        if (fez)
        {
            Connection *janela = new Connection(selected, this);
            int result = janela->exec();
            if (result == QDialog::Accepted) {
                refresh_connections();
            }
        }
    }
}


void MainWindow::on_buttonUpdateSchemas_clicked()
{
    refresh_schemas(actual_host);
}


void MainWindow::on_buttonUpdateTables_clicked()
{
    refresh_tables(actual_host, actual_schema);
}

void MainWindow::on_buttonFilterSchemas_triggered(QAction *arg1)
{

}


void MainWindow::on_actionQuit_triggered()
{
    this->close();
}


void MainWindow::on_buttonNewConns_clicked()
{
    on_actionNew_connection_triggered();
}


