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
#include <QTextStream>
#include <QFile>
#include <QPainter>
#include <QPixmap>
#include <QFormLayout>

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
#include <statistics.h>
#include <users.h>
#include <batch.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;

extern QString currentTheme;

extern QString actual_host;
extern QString actual_schema;
extern QString actual_table;
extern QString actual_color;

extern bool prefLoaded;

extern QJsonArray colors;
extern QJsonArray colorThemes;

int newConnectionCount = 0;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->actionNew_schema->setDisabled(true);
    ui->actionNew_table->setDisabled(true);
    ui->actionUsers->setDisabled(true);
    ui->actionInformation->setDisabled(true);
    ui->actionStatistics->setDisabled(true);
    ui->actionProcesses->setDisabled(true);

    if (openPreferences())
    {
        currentTheme = getStringPreference("theme");
        if (currentTheme == "")
        {
            currentTheme = "light";
        }
        changeTheme();

        ui->listViewConns->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->listViewConns, &QListView::customContextMenuRequested,
                this, &MainWindow::mostrarMenuContextoConns);

        ui->listViewSchemas->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->listViewSchemas, &QListView::customContextMenuRequested,
                this, &MainWindow::mostrarMenuContextoSchemas);

        ui->listViewTables->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->listViewTables, &QListView::customContextMenuRequested,
                this, &MainWindow::mostrarMenuContextoTables);

        refresh_connections();
        getPreferences();

    } else {
        customAlert("Error","Can't create preferences file! Check permissions and try again...");
        QTimer::singleShot(0, qApp, &QApplication::quit);
    }
    ui->statusbar->showMessage("Version: " + QString(APP_VERSION) + " - Build:" + QString(APP_BUILD_DATE) + " " + QString(APP_BUILD_TIME));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeTheme()
{
    qDebug() << "mudar tema: " << currentTheme;
    QFile f(QString(":themes/%1/%2style.qss").arg(currentTheme).arg(currentTheme));

    if (!f.exists())   {
        printf("Unable to set stylesheet, file not found\n");
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        this->setStyleSheet(ts.readAll());

        for (const QJsonValue &val : colorThemes) {
            QJsonObject obj = val.toObject();
            if (obj["theme"].toString() == currentTheme) {
                colors = obj["colors"].toArray();
                break;
            }
        }
        setStringPreference("theme", currentTheme);

        if (currentTheme=="dark")
        {
            QPixmap pixmap(10, 10);
            pixmap.fill(Qt::darkGray); // cor de fundo

            QPainter painter(&pixmap);
            painter.setPen(Qt::black);
            painter.drawPoint(5, 5); // ponto central
            painter.end();
            ui->mdiArea->setBackground(QBrush(pixmap));
        } else {
            QPixmap pixmap(10, 10);
            pixmap.fill(Qt::white); // cor de fundo

            QPainter painter(&pixmap);
            painter.setPen(Qt::gray);
            painter.drawPoint(5, 5); // ponto central
            painter.end();
            ui->mdiArea->setBackground(QBrush(pixmap));
        }

    }
}

void MainWindow::customAlert(QString title, QString message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setStyleSheet(
        "QLabel {"
        "    padding: 10px;"
        "   min-height: 30px;"
        "   min-width: 300px;"
        "}"
        "QPushButton {"
        "    padding: 5px;"
        "    margin: 5px"
        "}"
        );
    msgBox.exec();
}


void MainWindow::createDatabaseDialog(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Create database");

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *editNome = new QLineEdit();
    QLineEdit *editCharset = new QLineEdit();
    QLineEdit *editCollate = new QLineEdit();

    editCharset->setText("utf8mb4");
    editCollate->setText("utf8mb4_general_ci");

    formLayout->addRow("Database name:", editNome);
    formLayout->addRow("Character Set:", editCharset);
    formLayout->addRow("Collation:", editCollate);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString nome = editNome->text().trimmed();
        QString charset = editCharset->text().trimmed();
        QString collate = editCollate->text().trimmed();

        if (nome.isEmpty()) {
            customAlert("Error", "");
            customAlert("Error", "Database name is mandatory");
            return;
        }

        QString sql = QString("CREATE DATABASE `%1` CHARACTER SET %2 COLLATE %3;")
                          .arg(nome, charset, collate);
        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + actual_host));
        if (!query.exec(sql)) {
            customAlert("Create database error", query.lastError().text());
        } else {
            refresh_schemas(actual_host, false);
            // customAlert("Success", "Database created!");
        }
    }
}


void MainWindow::createTableDialog(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Create table");

    QFormLayout *formLayout = new QFormLayout(&dialog);

    QLineEdit *editName = new QLineEdit();


    formLayout->addRow("Table name:", editName);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    formLayout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = editName->text().trimmed();

        if (name.isEmpty()) {
            customAlert("Error", "");
            customAlert("Error", "Table name is mandatory");
            return;
        }

        QString sql = QString("CREATE TABLE %1 ( id INT AUTO_INCREMENT PRIMARY KEY )").arg(name);
        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + actual_host));
        if (!query.exec(sql)) {
            customAlert("Create table error", query.lastError().text());
        } else {
            refresh_tables(actual_host);
            // customAlert("Success", "Database created!");
        }
    }
}


bool MainWindow::host_connect(QString selectedHost)
{
    QJsonObject item = getConnection(selectedHost);

    // qDebug() << "Tentando conexao...";
    // qDebug() << " Host: " << item["host"].toVariant().toString();
    // qDebug() << " User: " << item["user"].toVariant().toString();
    // qDebug() << " Pass: " << item["pass"].toVariant().toString();

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

        customAlert("Connection failed!", "Check parameters and try again");
        return(false);
    } else {
        ui->statusbar->showMessage("Host connected!");
        refresh_schemas(selectedHost, true);
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

void MainWindow::refresh_schemas(QString selectedHost, bool jumpToTables)
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

        if (query.exec("SHOW DATABASES"))
        {
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
                // refresh_schema(actual_first_schema);
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
                if (jumpToTables)
                {
                    refresh_tables(selectedHost);
                }
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
    qDebug() << index;
}

void MainWindow::on_listViewConns_doubleClicked(const QModelIndex &index)
{
    if (ui->buttonEditConns->isChecked())
    {
        listViewConns_edit(index);
    } else {
        ui->actionNew_schema->setDisabled(false);
        ui->actionUsers->setDisabled(false);
        listViewConns_open(index);
    }
}



void MainWindow::on_listViewSchemas_clicked(const QModelIndex &index)
{
    actual_schema = index.data(Qt::DisplayRole).toString();

    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + actual_host);

    QSqlQuery query(db);

    QString consulta = "SELECT table_schema AS name, ROUND(SUM(data_length + index_length) / 1024 / 1024, 2) size "
                       "FROM information_schema.TABLES "
                       "WHERE table_schema = '"+actual_schema+"' "
                                         "GROUP BY table_schema ";
    if (query.exec( consulta )) {
        if (query.next()) {
            ui->labelSchemaSize->setText(query.value("size").toString() + " Mb");
        }
    }

    consulta = "SELECT COUNT(*) AS total_tables FROM information_schema.tables "
               "WHERE table_schema = '"+actual_schema+"'";
    if (query.exec( consulta )) {
        if (query.next()) {
            ui->labelSchemaTables->setText(query.value("total_tables").toString());
        }
    }
}


void MainWindow::on_listViewSchemas_doubleClicked(const QModelIndex &index)
{
    actual_schema = index.data(Qt::DisplayRole).toString();
    ui->actionNew_table->setDisabled(false);
    refresh_schema(actual_schema);
}


void MainWindow::on_listViewTables_clicked(const QModelIndex &index)
{
    actual_table= index.data(Qt::DisplayRole).toString();

    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + actual_host);

    QSqlQuery query(db);

    QString consulta = "SELECT table_name AS 'table', table_rows AS 'rows',"
        "ROUND((data_length + index_length) / 1024 / 1024, 2) AS 'size'"
        "FROM  information_schema.tables WHERE  table_schema = '"+actual_schema+"' AND "
        " table_name = '"+actual_table+"'";
    if (query.exec( consulta )) {
        if (query.next()) {
            ui->labelTablesSize->setText(query.value("size").toString() + " Mb");
            ui->labelTablesRows->setText(query.value("rows").toString());
        }
    }
}


void MainWindow::on_listViewTables_doubleClicked(const QModelIndex &index)
{
    // actual_table= index.data(Qt::DisplayRole).toString();
    if (ui->buttonEditTables->isChecked())
    {
        on_listViewTables_edit(index);
    } else {
        on_listViewTables_open(index);
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
    refresh_schemas(actual_host, false);
}


void MainWindow::on_buttonUpdateSchemas_clicked()
{
    refresh_schemas(actual_host, false);
}


void MainWindow::on_buttonUpdateTables_clicked()
{
    refresh_tables(actual_host);
}

void MainWindow::on_buttonNewConns_clicked()
{
    on_actionNew_connection_triggered();
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
    // QApplication::quit();
}

void MainWindow::on_actionTile_triggered()
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionCascade_triggered()
{
    ui->mdiArea->cascadeSubWindows();
}


void MainWindow::on_actionTheme_triggered()
{
    if (currentTheme == "light")
    {
        currentTheme = "dark";
    } else {
        currentTheme = "light";
    }
    changeTheme();
    refresh_connections();
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
    QAction *schemaBatchRun = menu.addAction("Batch run");

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
    else if (selectedAction == schemaBatchRun) {
        batch_run();
    }

}


void MainWindow::mostrarMenuContextoSchemas(const QPoint &pos)
{
    QModelIndex index = ui->listViewSchemas->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction *schemaOpen = menu.addAction("Open");
    QAction *schemaCreate = menu.addAction("Create");
    QAction *schemaDrop = menu.addAction("Drop");
    menu.addSeparator();
    QAction *schemaUsers = menu.addAction("Users");
    QAction *schemaStatistics = menu.addAction("Statistics");
    menu.addSeparator();
    QAction *schemaBackup = menu.addAction("Backup");
    QAction *schemaRestore = menu.addAction("Restore");
    QAction *schemaBatchRun = menu.addAction("Batch run");

    QAction *selectedAction = menu.exec(ui->listViewSchemas->viewport()->mapToGlobal(pos));

    if (selectedAction == schemaOpen) {
        on_listViewSchemas_clicked(index);
    }
    else if (selectedAction == schemaStatistics) {
        actual_schema = index.data(Qt::DisplayRole).toString();
        Statistics *janela = new Statistics(actual_host, actual_schema, this);
        janela->exec();
    }
    else if (selectedAction == schemaBatchRun) {
        batch_run();
    }
    else if (selectedAction == schemaCreate) {
        createDatabaseDialog(this);
    }
    else if (selectedAction == schemaDrop) {
        QString dbName = index.data(Qt::DisplayRole).toString();

        QMessageBox msgBox;
        msgBox.setStyleSheet(
            "QLabel:last {"
            "   padding-right: 10px;"
            "   min-height: 30px;"
            "   min-width: 280px;"
            "}"
            "QPushButton {"
            "    padding: 4px;"
            "    margin: 20px;"
            "    min-height: 16px;"
            "}"
            );
        msgBox.setWindowTitle("Confirm Deletion");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        QString message = QString("Are you sure you want to drop database\n\"%1\"?").arg(dbName);
        msgBox.setText(message);

        // Mostra a caixa de mensagem e captura a resposta
        QMessageBox::StandardButton reply = static_cast<QMessageBox::StandardButton>(msgBox.exec());

        if (reply == QMessageBox::Yes) {
            QSqlQuery query(QSqlDatabase::database("mysql_connection_" + actual_host));
            QString sql = QString("DROP DATABASE `%1`;").arg(dbName);

            if (!query.exec(sql)) {
                customAlert("Error", "Failed to delete database:\n" + query.lastError().text());
            } else {
                refresh_schemas(actual_host, false);
                // customAlert("Success", "Database deleted successfully.");

            }
        }
    }
}

void MainWindow::on_listViewTables_open(const QModelIndex &index)
{
    actual_table= index.data(Qt::DisplayRole).toString();
    QMdiSubWindow *prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }

    Sql *form = new Sql(actual_host, actual_schema, actual_table, actual_color);

    QMdiSubWindow *sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);  // subjanela será destruída ao fechar
    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();

}

void MainWindow::on_listViewTables_edit(const QModelIndex &index)
{
    actual_table= index.data(Qt::DisplayRole).toString();
    QMdiSubWindow *prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }
    Structure *form = new Structure(actual_host, actual_schema, actual_table);

    QMdiSubWindow *sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);  // subjanela será destruída ao fechar

    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();
}

void MainWindow::mostrarMenuContextoTables(const QPoint &pos)
{
    QModelIndex index = ui->listViewTables->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction *tableOpen = menu.addAction("Open");
    QAction *tableEdit = menu.addAction("Edit");
    QAction *tableCreate = menu.addAction("Create");
    QAction *tableDrop = menu.addAction("Drop");
    menu.addSeparator();
    QAction *tableExportSQL = menu.addAction("Export as SQL");
    QAction *tableExportCSV = menu.addAction("Export as CSV");
    QAction *tableExportPDF = menu.addAction("Export as PDF");

    QAction *selectedAction = menu.exec(ui->listViewTables->viewport()->mapToGlobal(pos));

    if (selectedAction == tableOpen) {
        on_listViewTables_open(index); // TODO
    }
    else if (selectedAction == tableEdit) {
        on_listViewTables_edit(index); // TODO
    }
    else if (selectedAction == tableCreate) {
        createTableDialog(this);
    }
    else if (selectedAction == tableDrop) {
        QString dbTable = index.data(Qt::DisplayRole).toString();

        QMessageBox msgBox;
        msgBox.setStyleSheet(
            "QLabel:last {"
            "   padding-right: 10px;"
            "   min-height: 30px;"
            "   min-width: 200px;"
            "}"
            "QPushButton {"
            "    padding: 10px;"
            "    margin: 10px;"
            "    min-height: 16px;"
            "}"
            );
        msgBox.setWindowTitle("Confirm Delete");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        QString message = QString("Are you sure you want to drop table\n\"%1\"?").arg(dbTable);
        msgBox.setText(message);

        QMessageBox::StandardButton reply = static_cast<QMessageBox::StandardButton>(msgBox.exec());

        if (reply == QMessageBox::Yes) {
            QSqlQuery query(QSqlDatabase::database("mysql_connection_" + actual_host));
            QString sql = QString("DROP TABLE `%1`;").arg(dbTable);

            if (!query.exec(sql)) {
                customAlert("Error", "Failed to delete table:\n" + query.lastError().text());
            } else {
                refresh_tables(actual_host);
                // customAlert("Success", "Table deleted successfully.");

            }
        }
    }

}




void MainWindow::on_actionNew_schema_triggered()
{
    if (actual_host != "")
    {
        createDatabaseDialog(this);
    }
}


void MainWindow::on_actionNew_table_triggered()
{
    if (actual_host != "")
    {
        createTableDialog(this);
    }
}


void MainWindow::on_actionUsers_triggered()
{
    QMdiSubWindow *prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }

    Users *form = new Users(actual_host, actual_schema, this);

    QMdiSubWindow *sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);  // subjanela será destruída ao fechar
    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();

}

void MainWindow::batch_run()
{
    QMdiSubWindow *prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }

    Batch *form = new Batch(actual_host, actual_schema, this);

    QMdiSubWindow *sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);  // subjanela será destruída ao fechar
    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();


}
