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
#include <QProgressDialog>
#include <QTimer>
#include <QAction>
#include <QContextMenuEvent>
#include <QMenu>
#include <QInputDialog>

#include <QSortFilterProxyModel>
#include <QMap>
#include <QProcess>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include <functions.h>
#include <connection.h>
#include <sql.h>
#include <structure.h>
#include <tunnelsqlmanager.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;

extern QString actual_host;
extern QString actual_schema;
extern QString actual_table;
extern QString actual_color;

int newConnectionCount = 0;



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    openPreferences();

    ui->setupUi(this);

    // Menus de contexto
    ui->listViewConns->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listViewSchemas->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->listViewTables->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->listViewConns, &QListView::customContextMenuRequested,
            this, &MainWindow::mostrarMenuContextoConns);

    connect(ui->listViewSchemas, &QListView::customContextMenuRequested,
            this, &MainWindow::mostrarMenuContextoSchemas);

    connect(ui->listViewTables, &QListView::customContextMenuRequested,
            this, &MainWindow::mostrarMenuContextoTables);

    refresh_connections();
    getPreferences();

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

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();
    dbMysql = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection_"+selectedHost);

    if (item["ssh_host"] != "")
    {
        // QProgressDialog progress("Conectando...", "Cancelar", 0, 0, this);
        // progress.setWindowModality(Qt::ApplicationModal);
        // progress.setCancelButton(nullptr);
        // progress.show();

        TunnelSqlManager *manager = new TunnelSqlManager(this);

        bool ok = manager->conectar(
            item["name"].toVariant().toString(),
            item["ssh_user"].toVariant().toString(),
            item["ssh_host"].toVariant().toString(),
            item["ssh_port"].toVariant().toString(),
            item["ssh_pass"].toVariant().toString(),
            item["ssh_keyfile"].toVariant().toString(),
            item["host"].toVariant().toString(),
            item["port"].toVariant().toString(),
            item["user"].toVariant().toString(),
            item["pass"].toVariant().toString(),
            item["schema"].toVariant().toString());
        // progress.close();
        if (!ok) {
            QApplication::restoreOverrideCursor();
            QApplication::processEvents();
            return(false);
            // dbMysql = manager->obterConexao(item["name"].toVariant().toString());
            // manager->obterConexao(item["name"].toVariant().toString());
        }
    } else {
        dbMysql.setHostName(item["host"].toVariant().toString());
        dbMysql.setDatabaseName(item["schema"].toVariant().toString());
        dbMysql.setPort(item["port"].toVariant().toInt());
        dbMysql.setUserName(item["user"].toVariant().toString());
        dbMysql.setPassword(item["pass"].toVariant().toString());
    }
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
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
    return(true);
}

void MainWindow::refresh_connections() {
    ui->toolBoxLeft->setCurrentIndex(2);
    ui->lineEditConns->setText("");

    QStandardItemModel *modelo = new QStandardItemModel(this);

    QIcon iconeConexao;
    if (ui->buttonEditConns->isChecked())
    {
        iconeConexao = QIcon(":/icons/resources/cloud connection 2.svg");
    } else {
        iconeConexao = QIcon(":/icons/resources/cloud connection.svg");
    }

    for (const QJsonValue &valor : connections) {
        if (valor.isObject()) {
            QJsonObject item = valor.toObject();
            QString nome = item["name"].toString();
            QString corDeFundo = item["color"].toString().toLower();

            QStandardItem *linha = new QStandardItem(iconeConexao, nome);  // Ícone aplicado aqui

            // Define a cor de fundo com base no texto
            linha->setBackground(QColor(getRgbFromColorName(corDeFundo)));
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
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QApplication::processEvents();
        QJsonObject item = getConnection(selectedHost);
        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + selectedHost));

        if (query.exec("SHOW DATABASES")) {
            ui->lineEditSchemas->setText("");
            QStandardItemModel *modelo = new QStandardItemModel(this);
            int sel = -1;
            int cnt = 0;

            QIcon iconeBanco1;
            QIcon iconeBanco2;
            if (ui->buttonEditSchemas->isChecked())
            {
                iconeBanco1 = QIcon(":/icons/resources/box.svg");
                iconeBanco2 = QIcon(":/icons/resources/box 3.svg");
            } else {
                iconeBanco1 = QIcon(":/icons/resources/box.svg");
                iconeBanco2 = QIcon(":/icons/resources/box 2.svg");
            }

            QString actual_first_schema = "";
            while (query.next()) {
                QString name = query.value(0).toString();
                if (name == "mysql" || name=="information_schema" || name=="performance_schema"  || name=="sys")
                {
                    QStandardItem *linha = new QStandardItem(iconeBanco1, name);
                    modelo->appendRow(linha);
                } else {
                    QStandardItem *linha = new QStandardItem(iconeBanco2, name);
                    modelo->appendRow(linha);
                    if (actual_first_schema == "")
                    {
                        actual_first_schema = name;
                        sel = cnt;
                    }
                }

                if (item["schema"].toString() == name) {
                    sel = cnt;
                }
                cnt++;
            }

            if (item["schema"].toString() == "")
            {
                refresh_schema(actual_first_schema);
            }

            // Proxy de filtro
            QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
            proxy->setSourceModel(modelo);
            proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

            ui->listViewSchemas->setModel(proxy);

            // Filtro no LineEdit
            connect(ui->lineEditSchemas, &QLineEdit::textChanged, this, [=](const QString &texto) {
                QString pattern = QString("(%1)").arg(texto);
                QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
                proxy->setFilterRegularExpression(re);
            });

            if (sel > -1) {
                QModelIndex index = proxy->index(sel, 0);
                ui->listViewSchemas->setCurrentIndex(index);
                QApplication::restoreOverrideCursor();
                QApplication::processEvents();
                refresh_tables(selectedHost);
            }



        }
        QApplication::restoreOverrideCursor();
        QApplication::processEvents();
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
        QSqlQuery query(QSqlDatabase::database("mysql_connection_"+actual_host));

        if (query.exec("USE "+selectedSchema)) {
            refresh_tables(actual_host);
            ui->toolBoxLeft->setCurrentIndex(0);
            ui->buttonFilterTables->setChecked(false);
        }
    }
}

void MainWindow::refresh_tables(QString selectedHost) {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + selectedHost));
    if (query.exec("SHOW TABLES")) {
        ui->lineEditTables->setText("");

        QStandardItemModel *modelo = new QStandardItemModel(this);
        QIcon iconeTabela;
        if (ui->buttonEditTables->isChecked())
        {
            iconeTabela = QIcon(":/icons/resources/size.svg");
        } else {
            iconeTabela = QIcon(":/icons/resources/copy.svg");
        }

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
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
}

void MainWindow::on_listViewConns_clicked(const QModelIndex &index)
{
    // actual_host = index.data(Qt::DisplayRole).toString();

    if (ui->buttonEditConns->isChecked())
    {
        listViewConns_edit(index);
        // if (index.isValid()) {
        //     Connection *janela = new Connection(actual_host, this);
        //     int result = janela->exec();
        //     if (result == QDialog::Accepted) {
        //         refresh_connections();
        //     }
        // }
    } else {
        listViewConns_open(index);
        // QModelIndex i = ui->listViewConns->currentIndex();
        // if (i.isValid()) {
        //     if (host_connect(actual_host))
        //     {
        //         ui->buttonFilterSchemas->setChecked(false);
        //         ui->buttonFilterTables->setChecked(false);
        //         ui->toolBoxLeft->setCurrentIndex(1);
        //         QJsonObject item = getConnection(actual_host);
        //         actual_color = item["color"].toString();
        //     }
        // }
    }
}


void MainWindow::on_listViewSchemas_clicked(const QModelIndex &index)
{
    actual_schema = index.data(Qt::DisplayRole).toString();
    refresh_schema(actual_schema);
}

void MainWindow::on_listViewTables_clicked(const QModelIndex &index)
{
    actual_table= index.data(Qt::DisplayRole).toString();

    if (ui->buttonEditTables->isChecked())
    {
        Structure *form = new Structure(actual_host, actual_schema, actual_table);

        QMdiSubWindow *sub = new QMdiSubWindow;
        sub->setWidget(form);
        sub->setAttribute(Qt::WA_DeleteOnClose);  // subjanela será destruída ao fechar
        ui->mdiArea->addSubWindow(sub);
        sub->resize(500, 360);
        sub->showMaximized();

    } else {
        Sql *form = new Sql(actual_host, actual_schema, actual_table, actual_color);

        QMdiSubWindow *sub = new QMdiSubWindow;
        sub->setWidget(form);
        sub->setAttribute(Qt::WA_DeleteOnClose);  // subjanela será destruída ao fechar
        ui->mdiArea->addSubWindow(sub);
        sub->resize(500, 360);
        sub->showMaximized();
    }
}


void MainWindow::on_buttonFilterSchemas_clicked()
{
    QString pref_name = "fav_"+actual_host;

    if (ui->buttonFilterSchemas->isChecked())
    {
        if (ui->lineEditSchemas->text() == "")
        {
            QString value = getStringPreference(pref_name);
            if (value != "")
            {
                ui->lineEditSchemas->setText(value);
            }
        } else {
            setStringPreference(pref_name, ui->lineEditSchemas->text());
        }
    } else {
        ui->lineEditSchemas->setText("");
    }
}


void MainWindow::on_buttonEditConns_clicked()
{
    refresh_connections();
}


void MainWindow::on_buttonEditTables_clicked()
{
    refresh_tables(actual_host);
}


void MainWindow::on_buttonEditSchemas_clicked()
{
    refresh_schemas(actual_host);
}


void MainWindow::on_buttonUpdateSchemas_clicked()
{
    refresh_schemas(actual_host);
}


void MainWindow::on_buttonUpdateTables_clicked()
{
    refresh_tables(actual_host);
}

void MainWindow::on_buttonNewConns_clicked()
{
    on_actionNew_connection_triggered();
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

void MainWindow::on_actionQuit_triggered()
{
    dbMysql.close();
    dbPreferences.close();
    this->close();
}

void MainWindow::on_actionTile_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionCascade_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}



void MainWindow::on_buttonFilterTables_clicked()
{
    QString pref_name = "fav_"+actual_host+"_"+actual_schema;

    if (ui->buttonFilterTables->isChecked())
    {
        if (ui->lineEditTables->text() == "")
        {
            QString value = getStringPreference(pref_name);
            if (value != "")
            {
                ui->lineEditTables->setText(value);
            }
        } else {
            setStringPreference(pref_name, ui->lineEditTables->text());
        }
    } else {
        ui->lineEditTables->setText("");
    }

}


void MainWindow::listViewConns_open(const QModelIndex &index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    if (host_connect(actual_host))
    {
        ui->buttonFilterSchemas->setChecked(false);
        ui->buttonFilterTables->setChecked(false);
        ui->toolBoxLeft->setCurrentIndex(1);
        QJsonObject item = getConnection(actual_host);
        actual_color = item["color"].toString();
    }
}

void MainWindow::listViewConns_edit(const QModelIndex &index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    Connection *janela = new Connection(actual_host, this);
    int result = janela->exec();
    if (result == QDialog::Accepted) {
        refresh_connections();
    }
}

void MainWindow::listViewConns_clone(const QModelIndex &index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    QJsonObject orig = getConnection(actual_host);
    QString selected ;
    bool fez = false;
    newConnectionCount = 0;
    while(!fez)
    {
        selected = actual_host;
        if (newConnectionCount > 0) {
            selected+=" " +std::to_string(newConnectionCount);
        }
        newConnectionCount++;
        fez = addConnection(selected,
                            orig["color"].toVariant().toString(),
                            orig["host"].toVariant().toString(),
                            orig["user"].toVariant().toString(),
                            orig["pass"].toVariant().toString(),
                            orig["port"].toVariant().toString(),
                            orig["ssh_host"].toVariant().toString(),
                            orig["ssh_user"].toVariant().toString(),
                            orig["ssh_pass"].toVariant().toString(),
                            orig["ssh_port"].toVariant().toString(),
                            orig["ssh_keyfile"].toVariant().toString() );
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

void MainWindow::listViewConns_remove(const QModelIndex &index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    deleteConnection(actual_host);
    refresh_connections();
}

void MainWindow::mostrarMenuContextoConns(const QPoint &pos)
{
    QModelIndex index = ui->listViewConns->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction *schemaOpen = menu.addAction("Open");
    QAction *schemaEdit = menu.addAction("Edit");
    QAction *schemaClone = menu.addAction("Clone");
    QAction *schemaRemove = menu.addAction("Remove");

    QAction *selectedAction = menu.exec(ui->listViewConns->viewport()->mapToGlobal(pos));

    if (selectedAction == schemaOpen) {
        listViewConns_open(index);
    }
    else if (selectedAction == schemaEdit) {
        listViewConns_edit(index);
    }
    else if (selectedAction == schemaClone) {
        listViewConns_clone(index);
    }
    else if (selectedAction == schemaRemove) {
        listViewConns_remove(index);
    }
}


void MainWindow::mostrarMenuContextoSchemas(const QPoint &pos)
{
    QModelIndex index = ui->listViewSchemas->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction *schemaOpen = menu.addAction("Open");
    QAction *schemaStatistics = menu.addAction("Statistics");
    QAction *schemaBatchRun = menu.addAction("Batch run");

    QAction *selectedAction = menu.exec(ui->listViewSchemas->viewport()->mapToGlobal(pos));

    if (selectedAction == schemaOpen) {
        on_listViewSchemas_clicked(index);
    }
    else if (selectedAction == schemaStatistics) {
    // TODO
    }
    else if (selectedAction == schemaBatchRun) {
    // TODO
    }
}

void MainWindow::mostrarMenuContextoTables(const QPoint &pos)
{
    QModelIndex index = ui->listViewTables->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction *tableOpen = menu.addAction("Open");
    QAction *tableEdit = menu.addAction("Edit");

    QAction *selectedAction = menu.exec(ui->listViewTables->viewport()->mapToGlobal(pos));

    if (selectedAction == tableOpen) {
        on_listViewTables_clicked(index); // TODO
    }
    else if (selectedAction == tableEdit) {
        on_listViewTables_clicked(index); // TODO
    }
}
