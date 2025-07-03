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
#include <QStandardItemModel>
#include <QMdiSubWindow>

#include <QSortFilterProxyModel>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <functions.h>
#include <connection.h>
#include <sql.h>

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

    qDebug() << "Tentando conexao...";
    qDebug() << " Host: " << item["host"].toVariant().toString();
    qDebug() << " User: " << item["user"].toVariant().toString();
    qDebug() << " Pass: " << item["pass"].toVariant().toString();

    dbMysql = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection_"+selectedHost);

    dbMysql.setHostName(item["host"].toVariant().toString());
    dbMysql.setDatabaseName(item["schema"].toVariant().toString());
    dbMysql.setPort(item["port"].toVariant().toInt());
    dbMysql.setUserName(item["user"].toVariant().toString());
    dbMysql.setPassword(item["pass"].toVariant().toString());

    if (!dbMysql.open()) {
        qDebug() << dbMysql.lastError();
        ui->statusbar->showMessage("Database connection failed!");
        QMessageBox::information(this,
                                 "Connection failed!",
                                 "Check parameters and try again",
                                 QMessageBox::Ok
                                 );
        return(false);
    } else {
        ui->statusbar->showMessage("Host connected!");
        refresh_schemas(selectedHost);
    }
    qDebug() << dbMysql.lastError();
    return(true);
}

void MainWindow::refresh_connections() {
    ui->toolBoxLeft->setCurrentIndex(2);
    ui->lineEditConns->setText("");

    QStandardItemModel *modelo = new QStandardItemModel(this);
    QIcon iconeConexao(":/icons/resources/cloud sunny.svg");  // Ícone do arquivo de recursos .qrc

    for (const QJsonValue &valor : connections) {
        if (valor.isObject()) {
            QJsonObject item = valor.toObject();
            QString nome = item["name"].toString();
            QString corDeFundo = item["color"].toString().toLower();

            QStandardItem *linha = new QStandardItem(iconeConexao, nome);  // Ícone aplicado aqui

            // Define a cor de fundo com base no texto
            if (corDeFundo == "blue")       linha->setBackground(QColor("#D6EAF8"));
            else if (corDeFundo == "yellow")linha->setBackground(QColor("#FCF3CF"));
            else if (corDeFundo == "orange")linha->setBackground(QColor("#FDEBD0"));
            else if (corDeFundo == "red")   linha->setBackground(QColor("#FADBD8"));
            else if (corDeFundo == "brown") linha->setBackground(QColor("#F6DDCC"));
            else if (corDeFundo == "purple")linha->setBackground(QColor("#EBDEF0"));
            else if (corDeFundo == "green") linha->setBackground(QColor("#D5F5E3"));
            else if (corDeFundo == "grey")  linha->setBackground(QColor("#E5E8E8"));

            modelo->appendRow(linha);
        }
    }

    // Proxy de filtro
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(modelo);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listViewConns->setModel(proxy);

    // Filtro no LineEdit
    connect(ui->lineEditConns, &QLineEdit::textChanged, this, [=](const QString &texto) {
        QString pattern = QString("(%1)").arg(texto);
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        proxy->setFilterRegularExpression(re);
    });

    ui->statusbar->showMessage("Connections updated");
}


// void MainWindow::refresh_connections() {
//     ui->toolBoxLeft->setCurrentIndex(2);
//     ui->lineEditConns->setText("");

//     QStandardItemModel *modelo = new QStandardItemModel(this);

//     for (const QJsonValue &valor : connections) {
//         if (valor.isObject()) {
//             QJsonObject item = valor.toObject();
//             QString nome = item["name"].toString();
//             QString corDeFundo = item["color"].toString().toLower();

//             QStandardItem *linha = new QStandardItem(nome);
//             // Define a cor de fundo com base no texto
//             if (corDeFundo == "blue")       linha->setBackground(QColor("#D6EAF8"));
//             else if (corDeFundo == "yellow")linha->setBackground(QColor("#FCF3CF"));
//             else if (corDeFundo == "orange")linha->setBackground(QColor("#FDEBD0"));
//             else if (corDeFundo == "red")   linha->setBackground(QColor("#FADBD8"));
//             else if (corDeFundo == "brown") linha->setBackground(QColor("#F6DDCC"));
//             else if (corDeFundo == "purple")linha->setBackground(QColor("#EBDEF0"));
//             else if (corDeFundo == "green") linha->setBackground(QColor("#D5F5E3"));
//             else if (corDeFundo == "grey")  linha->setBackground(QColor("#E5E8E8"));

//             modelo->appendRow(linha);
//         }
//     }

//     // Proxy de filtro
//     QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
//     proxy->setSourceModel(modelo);
//     proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

//     ui->listViewConns->setModel(proxy);

//     // Filtro no LineEdit
//     connect(ui->lineEditConns, &QLineEdit::textChanged, this, [=](const QString &texto) {
//         QString pattern = QString("(%1)").arg(texto);
//         QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
//         proxy->setFilterRegularExpression(re);
//     });
//     ui->statusbar->showMessage("Connections updated");
// }


void MainWindow::refresh_schemas(QString selectedHost)
{
    if (!dbMysql.open()) {
        ui->statusbar->showMessage("Connection failed!");
        QMessageBox::information(this,
                                 "Connection failed!",
                                 "Check parameters and try again",
                                 QMessageBox::Ok
                                 );
    } else {
        QJsonObject item = getConnection(selectedHost);
        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + selectedHost));

        if (query.exec("SHOW DATABASES")) {
            ui->lineEditSchemas->setText("");
            QStandardItemModel *modelo = new QStandardItemModel(this);
            int sel = -1;
            int cnt = 0;

            QIcon iconeBanco1(":/icons/resources/box.svg");
            QIcon iconeBanco2(":/icons/resources/box 2.svg");

            while (query.next()) {
                QString name = query.value(0).toString();
                if (name == "mysql" || name=="information_schema" || name=="performance_schema"  || name=="sys")
                {
                    QStandardItem *linha = new QStandardItem(iconeBanco1, name);
                    modelo->appendRow(linha);
                } else {
                    QStandardItem *linha = new QStandardItem(iconeBanco2, name);
                    modelo->appendRow(linha);
                }

                if (item["schema"].toString() == name) {
                    sel = cnt;
                }
                cnt++;
            }

            // Proxy de filtro
            QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
            proxy->setSourceModel(modelo);
            proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

            ui->listViewSchemas->setModel(proxy);

            connect(ui->lineEditSchemas, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);

            if (sel > -1) {
                QModelIndex index = proxy->index(sel, 0);
                ui->listViewSchemas->setCurrentIndex(index);
                refresh_tables(selectedHost);
            }
        }
    }
}

void MainWindow::refresh_schema(QString selectedSchema)
{
    if (!dbMysql.open()) {
        ui->statusbar->showMessage("Connection failed!");
        QMessageBox::information(this,
                                 "Connection failed!",
                                 "Check parameters and try again",
                                 QMessageBox::Ok
                                 );
    } else {
        // Consulta SQL
        QSqlQuery query(QSqlDatabase::database("mysql_connection_"+actual_host));

        if (query.exec("USE "+selectedSchema)) {
            refresh_tables(actual_host);
            ui->toolBoxLeft->setCurrentIndex(0);
        }
    }
}

void MainWindow::refresh_tables(QString selectedHost) {
    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + selectedHost));
    if (query.exec("SHOW TABLES")) {
        ui->lineEditTables->setText("");

        QStandardItemModel *modelo = new QStandardItemModel(this);
        QIcon iconeTabela(":/icons/resources/copy.svg");  // <- Ícone a ser exibido

        int sel = -1;
        int cnt = 0;

        while (query.next()) {
            QString name = query.value(0).toString();

            QStandardItem *linha = new QStandardItem(iconeTabela, name);
            modelo->appendRow(linha);

            if (false) {
                sel = cnt;
            }
            cnt++;
        }

        // Proxy de filtro
        QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(modelo);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

        ui->listViewTables->setModel(proxy);

        connect(ui->lineEditTables, &QLineEdit::textChanged, this, [=](const QString &texto) {
            QString pattern = QString("(%1)").arg(texto);
            QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
            proxy->setFilterRegularExpression(re);
        });

        if (sel > -1) {
            QModelIndex index = ui->listViewTables->model()->index(sel, 0);
            ui->listViewTables->setCurrentIndex(index);
        }

        ui->statusbar->showMessage("Tables updated");
    } else {
        ui->statusbar->showMessage("Find tables error!");
        qDebug() << "Erro ao buscar tabelas";
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
    refresh_tables(actual_host);
}

void MainWindow::on_actionQuit_triggered()
{
    dbMysql.close();
    dbPreferences.close();
    this->close();
}


void MainWindow::on_buttonNewConns_clicked()
{
    on_actionNew_connection_triggered();
}



void MainWindow::on_listViewTables_clicked(const QModelIndex &index)
{
    actual_table= index.data(Qt::DisplayRole).toString();

    if (true)
    {
        Sql *form = new Sql(actual_host, actual_schema, actual_table);

        QMdiSubWindow *sub = new QMdiSubWindow;
        sub->setWidget(form);
        sub->setAttribute(Qt::WA_DeleteOnClose);  // subjanela será destruída ao fechar
        ui->mdiArea->addSubWindow(sub);
        sub->resize(500, 360);
        sub->showMaximized();
    }
}


void MainWindow::on_toolBoxLeft_currentChanged(int index)
{
    if (index == 0)
    {
        ui->listViewConns->setFocus();
    }
    if (index == 1)
    {
        ui->listViewSchemas->setFocus();
    }
    if (index == 2)
    {
        ui->listViewTables->setFocus();
    }
}

