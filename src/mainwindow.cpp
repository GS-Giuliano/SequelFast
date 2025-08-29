#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <backup.h>
#include <restore.h>
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

extern QString sharedFavoriteDB;

extern bool prefLoaded;

extern QJsonArray colors;
extern QJsonArray colorThemes;

int newConnectionCount = 0;

QStringList favName;
QStringList favValue;


struct InterruptibleProgressDialog : public QDialog {
    bool& aborted;
    InterruptibleProgressDialog(bool& abortRef, QWidget* parent = nullptr)
        : QDialog(parent), aborted(abortRef) {
    }

protected:
    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            aborted = true;
            close();  // fecha visualmente a janela
        }
        else {
            QDialog::keyPressEvent(event);
        }
    }

    void closeEvent(QCloseEvent* event) override {
        aborted = true;
        QDialog::closeEvent(event);
    }
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("SequelFast - "+QCoreApplication::applicationVersion());

    ui->actionNew_schema->setDisabled(true);
    ui->actionNew_table->setDisabled(true);
    ui->actionUsers->setDisabled(true);
    ui->actionInformation->setDisabled(true);
    ui->actionStatistics->setDisabled(true);
    ui->actionProcesses->setDisabled(true);
    ui->actionBackup->setDisabled(true);
    ui->actionRestore->setDisabled(true);
    ui->toolBar->setStyleSheet("QToolButton { width: 80px;}");
    ui->treeViewFavorites->setAlternatingRowColors(true);

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
            this, &MainWindow::show_context_menu_Conns);

        ui->listViewSchemas->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->listViewSchemas, &QListView::customContextMenuRequested,
            this, &MainWindow::show_context_menu_Schemas);

        ui->listViewTables->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->listViewTables, &QListView::customContextMenuRequested,
            this, &MainWindow::show_context_menu_Tables);

        ui->treeViewFavorites->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->treeViewFavorites, &QListView::customContextMenuRequested,
            this, &MainWindow::show_context_menu_Favorites);


        refresh_connections();
        getPreferences();

        modelLog = new QStandardItemModel(0, 0, this);

        // ui->tableLogView->setModel(modelLog);
        ui->tableLogView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableLogView->horizontalHeader()->setStretchLastSection(true);

        modelLog->setColumnCount(4);
        modelLog->setHeaderData(0, Qt::Horizontal, "Date");
        modelLog->setHeaderData(1, Qt::Horizontal, "Host");
        modelLog->setHeaderData(2, Qt::Horizontal, "Schema");
        modelLog->setHeaderData(3, Qt::Horizontal, "Query");

        QFont fonte;
        fonte.setFamilies(QStringList()
                          << "Segoe UI"
                          << ".SF NS Text"
                          << "Ubuntu"
                          << "Cantarell"
                          << "Noto Sans"
                          << "Sans Serif");
        fonte.setPointSize(9);
        ui->tableLogView->setFont(fonte);
        // ui->tableLogView->setColumnWidth(0,52);
        // ui->tableLogView->setColumnWidth(1,42);
        // ui->tableLogView->setColumnWidth(2,42);


        // Carrega log
        QSqlQuery query(QSqlDatabase::database("pref_connection"));

        if (!query.exec("SELECT date,host,schema,query FROM logs ORDER BY id DESC LIMIT 1000")) {
            qCritical() << "Erro ao consultar dados:" << query.lastError().text();
        }
        else {
            QModelIndex lastIdx;
            while (query.next()) {
                int row = modelLog->rowCount();
                modelLog->insertRow(row);

                modelLog->setData(modelLog->index(row, 0), query.value(0).toString());
                modelLog->setData(modelLog->index(row, 1), query.value(1).toString());
                modelLog->setData(modelLog->index(row, 2), query.value(2).toString());
                modelLog->setData(modelLog->index(row, 3), query.value(3).toString());

                lastIdx = modelLog->index(row, 0); // guarda o último índice
            }
            // Seleciona e exibe a última linha
            if (lastIdx.isValid()) {
                ui->tableLogView->setSelectionBehavior(QAbstractItemView::SelectRows);
                ui->tableLogView->selectRow(lastIdx.row());                   // seleciona a linha inteira
                ui->tableLogView->setCurrentIndex(lastIdx);                   // move o cursor
                ui->tableLogView->scrollTo(lastIdx, QAbstractItemView::PositionAtBottom); // rola até ela
                ui->tableLogView->setFocus();
            }
        }

        QSortFilterProxyModel* proxyLog = new QSortFilterProxyModel(this);
        proxyLog->setSourceModel(modelLog);
        proxyLog->setFilterKeyColumn(-1);
        proxyLog->setFilterCaseSensitivity(Qt::CaseInsensitive);

        ui->tableLogView->setModel(proxyLog);
        ui->tableLogView->resizeColumnsToContents();

        connect(ui->lineEditLog, &QLineEdit::textChanged, this, [=](const QString& texto) {
            QString pattern = QString("(%1)").arg(texto);
            QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
            proxyLog->setFilterRegularExpression(re);
        });


    }
    else {
        customAlert("Error", "Can't create preferences file! Check permissions and try again...");
        QTimer::singleShot(0, qApp, &QApplication::quit);
    }
    QTimer::singleShot(2000, this, [=]() {
        refresh_favorites();
        });
    ui->statusbar->showMessage("Version: " + QString(APP_VERSION) + " - Build:" + QString(APP_BUILD_DATE) + " " + QString(APP_BUILD_TIME));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::changeTheme()
{
    // qDebug() << "mudar tema: " << currentTheme;
    QFile f(QString(":themes/%1/%2style.qss").arg(currentTheme).arg(currentTheme));

    if (!f.exists()) {
        printf("Unable to set stylesheet, file not found\n");
    }
    else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        this->setStyleSheet(ts.readAll());

        for (const QJsonValue& val : colorThemes) {
            QJsonObject obj = val.toObject();
            if (obj["theme"].toString() == currentTheme) {
                colors = obj["colors"].toArray();
                break;
            }
        }
        setStringPreference("theme", currentTheme);

        if (currentTheme == "dark")
        {
            QPixmap pixmap(10, 10);
            pixmap.fill(Qt::darkGray); // cor de fundo

            QPainter painter(&pixmap);
            painter.setPen(Qt::black);
            painter.drawPoint(5, 5); // ponto central
            painter.end();
            ui->mdiArea->setBackground(QBrush(pixmap));
        }
        else {
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


void MainWindow::createDatabaseDialog(QWidget* parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Create database");

    QFormLayout* formLayout = new QFormLayout(&dialog);

    QLineEdit* editNome = new QLineEdit();
    QLineEdit* editCharset = new QLineEdit();
    QLineEdit* editCollate = new QLineEdit();

    editCharset->setText("utf8mb4");
    editCollate->setText("utf8mb4_general_ci");

    formLayout->addRow("Database name:", editNome);
    formLayout->addRow("Character Set:", editCharset);
    formLayout->addRow("Collation:", editCollate);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
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
        }
        else {
            refresh_schemas(actual_host, false);
            // customAlert("Success", "Database created!");
        }
    }
}


void MainWindow::createTableDialog(QWidget* parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Create table");

    QFormLayout* formLayout = new QFormLayout(&dialog);

    QLineEdit* editName = new QLineEdit();


    formLayout->addRow("Table name:", editName);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
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
        }
        else {
            refresh_tables(actual_host);
            // customAlert("Success", "Database created!");
        }
    }
}


bool MainWindow::host_connect(QString selectedHost)
{
    QJsonObject item = getConnection(selectedHost);

    connectMySQL(selectedHost, this);

    if (!dbMysql.open()) {
        qDebug() << dbMysql.lastError();
        ui->statusbar->showMessage("Database connection failed!");

        customAlert("Connection failed!", "Check parameters and try again");
        return(false);
    }
    else {
        ui->statusbar->showMessage("Host connected!");
        refresh_schemas(selectedHost, true);
    }
    return(true);
}

void MainWindow::refresh_connections() {
    ui->toolBoxLeft->setCurrentIndex(4);
    ui->lineEditConns->setText("");

    QStandardItemModel* modelo = new QStandardItemModel(this);

    QIcon iconeConexao;
    if (ui->buttonEditConns->isChecked())
    {
        iconeConexao = QIcon(":/icons/resources/cloud connection 2.svg");
    }
    else {
        iconeConexao = QIcon(":/icons/resources/cloud connection.svg");
    }

    for (const QJsonValue& valor : connections) {
        if (valor.isObject()) {
            QJsonObject item = valor.toObject();
            QString nome = item["name"].toString();
            QString corDeFundo = item["color"].toString().toLower();

            // Verifica se eh uma conexao que possui favoritos compartilhados
            if (item["shared"].toString() == "1")
            {
                sharedFavoriteDB = nome;
                iconeConexao = QIcon(":/icons/resources/lovely.svg");

            }

            QStandardItem* linha = new QStandardItem(iconeConexao, nome);  // Ícone aplicado aqui

            // Define a cor de fundo com base no texto
            linha->setBackground(QColor(getRgbFromColorName(corDeFundo)));
            modelo->appendRow(linha);
        }
    }

    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(modelo);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->listViewConns->setModel(proxy);

    connect(ui->lineEditConns, &QLineEdit::textChanged, this, [=](const QString& texto) {
        QString pattern = QString("(%1)").arg(texto);
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        proxy->setFilterRegularExpression(re);
        });

    ui->statusbar->showMessage("Connections updated");
}

void MainWindow::refresh_schemas(QString selectedHost, bool jumpToTables)
{
    ui->statusbar->showMessage("Loading schemas...");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    if (!dbMysql.open()) {
        QApplication::restoreOverrideCursor();
        QApplication::processEvents();

        QMessageBox::information(this,
            "Connection failed!",
            "Check parameters and try again",
            QMessageBox::Ok
        );
    }
    else {
        ui->statusbar->showMessage("Connection failed!");

        QJsonObject item = getConnection(selectedHost);
        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + selectedHost));

        ui->statusbar->showMessage("Loading schemas...");
        if (query.exec("SHOW DATABASES"))
        {
            ui->lineEditSchemas->setText("");
            QStandardItemModel* modelo = new QStandardItemModel(this);
            int sel = -1;
            int cnt = 0;

            QIcon iconeBanco1;
            QIcon iconeBanco2;
            if (ui->buttonEditSchemas->isChecked())
            {
                iconeBanco1 = QIcon(":/icons/resources/box.svg");
                iconeBanco2 = QIcon(":/icons/resources/box 3.svg");
            }
            else {
                iconeBanco1 = QIcon(":/icons/resources/box.svg");
                iconeBanco2 = QIcon(":/icons/resources/box 2.svg");
            }

            QString actual_first_schema = "";
            while (query.next()) {
                QApplication::processEvents();
                QString name = query.value(0).toString();
                if (name == "_SequelFast" || name == "mysql" || name == "information_schema" || name == "performance_schema" || name == "sys")
                {
                    QStandardItem* linha = new QStandardItem(iconeBanco1, name);
                    modelo->appendRow(linha);
                }
                else {
                    QStandardItem* linha = new QStandardItem(iconeBanco2, name);
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

            QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
            proxy->setSourceModel(modelo);
            proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

            ui->listViewSchemas->setModel(proxy);

            connect(ui->lineEditSchemas, &QLineEdit::textChanged, this, [=](const QString& texto) {
                QString pattern = QString("(%1)").arg(texto);
                QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
                proxy->setFilterRegularExpression(re);
                });

            QApplication::restoreOverrideCursor();
            QApplication::processEvents();

            if (sel > -1) {
                QModelIndex index = proxy->index(sel, 0);
                ui->listViewSchemas->setCurrentIndex(index);
                if (jumpToTables)
                {
                    refresh_tables(selectedHost);
                }
            }
        }
        else {
            QApplication::restoreOverrideCursor();
            QApplication::processEvents();
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
    }
    else {
        QSqlQuery query(QSqlDatabase::database("mysql_connection_" + actual_host));

        if (query.exec("USE " + selectedSchema)) {
            refresh_tables(actual_host);
            ui->toolBoxLeft->setCurrentIndex(2);
            ui->buttonFilterTables->setChecked(false);
        }
    }
}

void MainWindow::refresh_favorites()
{
    ui->statusbar->showMessage("Loading favorites...");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    QString name = "";
    QString value = "";

    QSqlQuery query(QSqlDatabase::database("pref_connection"));
    query.prepare("SELECT name, value FROM prefs WHERE name LIKE :name ORDER BY name");
    query.bindValue(":name", "fav^%");
    if (!query.exec()) {
        qCritical() << "Erro:" << query.lastError().text();
        return;
    }

    auto* model = new QStandardItemModel(this);
    auto* group1 = new QStandardItem("Local");
    auto* group2 = new QStandardItem("Shared");
    auto* group3 = new QStandardItem(getUserName());
    group1->setEditable(false);
    group2->setEditable(false);
    group3->setEditable(false);
    // QFont f = group1->font(); f.setBold(true); group1->setFont(f);group2->setFont(f);group3->setFont(f);

    QIcon icone = ui->buttonEditFavorites->isChecked()
        ? QIcon(":/icons/resources/heart 2.svg")
        : QIcon(":/icons/resources/heart.svg");

    int sourceRowToSelect = -1;
    int row = 0;

    favName.clear();
    favValue.clear();

    // fav^host:schema:table:color:name
    while (query.next()) {
        name = query.value(0).toString();
        value = query.value(1).toString();
        const QStringList favList = name.split('^');

        const QString title = favList.value(5, name);
        // const QString bgColor = favList.value(4, "transparent");
        favName.append(name);
        favValue.append(value);

        auto* item = new QStandardItem(icone, title);
        item->setEditable(false);
        // item->setBackground(QColor(getRgbFromColorName(bgColor)));
        group1->appendRow(item);

        // se quiser selecionar o primeiro:
        if (sourceRowToSelect < 0) sourceRowToSelect = row;
        ++row;
        QApplication::processEvents();
    }

    model->appendRow(group1);

    bool sharedFavExists = false;
    bool sharedUserFavExists = false;
    // Shared favorites
    if (sharedFavoriteDB != "")
    {
        ui->statusbar->showMessage("Connecting shared favorites SequelFast schema...");
        QApplication::processEvents();
        QString connectionName = "mysql_connection_";
        if (connectMySQL(sharedFavoriteDB))
        {
            QApplication::processEvents();
            QSqlDatabase db = QSqlDatabase::database(connectionName + sharedFavoriteDB);
            if (!db.isValid()) {
                qDebug() << "Erro: Conexão com o banco de dados inválida para" << connectionName;
                return;
            }
            QSqlQuery query(db);


            // Verifica se a base de dados existe

            bool found = false;
            // qDebug() << "Favoritos compartilhados - Verificando se BD existe...";
            ui->statusbar->showMessage("Checking if SequelFast schema exists...");

            if (query.exec("SHOW DATABASES"))
            {
                QApplication::processEvents();
                while (query.next()) {
                    QString name = query.value(0).toString();
                    if (name == "_SequelFast")
                    {
                        found = true;
                    }
                }
            }

            if (!found)
            {
                ui->statusbar->showMessage("Not found. Creating database and table...");

                // qDebug() << "Favoritos compartilhados - Nao existe. Criando...";
                // Cria a tabela se não existir
                QString createDatabase = "CREATE DATABASE _SequelFast CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;";
                if (query.exec(createDatabase)) {
                    QApplication::processEvents();
                    if (query.exec("USE _SequelFast")) {
                        QString createTableSql = "CREATE TABLE IF NOT EXISTS prefs ("
                            "id INT AUTO_INCREMENT PRIMARY KEY,"
                            "name VARCHAR(255) NULL,"
                            "value TEXT NULL,"
                            "type VARCHAR(100) NULL"
                            ");";
                        if (!query.exec(createTableSql)) {
                            qCritical() << "Erro ao criar a tabela 'pref':" << query.lastError().text();
                        }
                    }
                }
            }
            else {
                ui->statusbar->showMessage("Loading shared favorites...");

                // qDebug() << "Favoritos compartilhados - Buscando favoritos...";
                QApplication::processEvents();
                if (query.exec("USE _SequelFast")) {
                    QApplication::processEvents();
                    if (query.exec("SELECT name,value FROM _SequelFast.prefs ORDER BY name")) {
                        while (query.next()) {
                            QApplication::processEvents();

                            sharedFavExists = true;
                            name = query.value(0).toString();
                            value = query.value(1).toString();

                            // qDebug() << "Name" << name;
                            // qDebug() << "Value" << value;

                            const QStringList favList = name.split('^');
                            QString title = favList.value(5, name);
                            QString user = favList.value(6);

                            bool IsGroup2 = true;
                            if (user != "")
                            {
                                // qDebug() << "Fav. privado" << user << getUserName();
                                if (user == getUserName())
                                {
                                    sharedUserFavExists = true;
                                    IsGroup2 = false;
                                }
                                else {
                                    continue;
                                }
                            }
                            else {
                            }

                            favName.append(name);
                            favValue.append(value);

                            auto* item = new QStandardItem(icone, title);
                            item->setEditable(false);

                            if (IsGroup2)
                                group2->appendRow(item);
                            else
                                group3->appendRow(item);

                            // se quiser selecionar o primeiro:
                            if (sourceRowToSelect < 0) sourceRowToSelect = row;
                            ++row;
                        }
                    }
                }

            }
        }
        else {
            qCritical() << "Nao foi possivel conectar a favoritos!" << sharedFavoriteDB;
        }

        if (sharedUserFavExists)
        {
            model->appendRow(group3);
        }
        if (sharedFavExists)
        {
            model->appendRow(group2);
        }
    }




    auto* proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    proxy->setRecursiveFilteringEnabled(true); // importante p/ filtrar filhos
#endif

    // supondo que você tem um QTreeView no UI:
    ui->treeViewFavorites->setModel(proxy);
    ui->treeViewFavorites->setHeaderHidden(true);
    ui->treeViewFavorites->expandAll(); // mostra os filhos

    connect(ui->lineEditFavorites, &QLineEdit::textChanged, this, [proxy](const QString& texto) {
        proxy->setFilterRegularExpression(QRegularExpression(texto, QRegularExpression::CaseInsensitiveOption));
        });

    if (sourceRowToSelect >= 0) {
        QModelIndex srcIdx = model->index(sourceRowToSelect, 0, group1->index());
        QModelIndex proxyIdx = proxy->mapFromSource(srcIdx);
        ui->treeViewFavorites->setCurrentIndex(proxyIdx);
    }

    ui->statusbar->showMessage("Favorites updated");
    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
}


void MainWindow::refresh_tables(QString selectedHost) {

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    ui->statusbar->showMessage("Connecting database...");

    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + selectedHost));
    ui->statusbar->showMessage("Loading tables...");
    if (query.exec("SHOW TABLES")) {
        ui->lineEditTables->setText("");

        QStandardItemModel* modelo = new QStandardItemModel(this);
        QIcon iconeTabela;
        if (ui->buttonEditTables->isChecked())
        {
            iconeTabela = QIcon(":/icons/resources/size.svg");
        }
        else {
            iconeTabela = QIcon(":/icons/resources/copy.svg");
        }

        int sel = -1;
        int cnt = 0;

        while (query.next()) {
            QApplication::processEvents();
            QString name = query.value(0).toString();

            QStandardItem* linha = new QStandardItem(iconeTabela, name);
            modelo->appendRow(linha);

            if (false) {
                sel = cnt;
            }
            cnt++;
        }

        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(modelo);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

        ui->listViewTables->setModel(proxy);

        connect(ui->lineEditTables, &QLineEdit::textChanged, this, [=](const QString& texto) {
            QString pattern = QString("(%1)").arg(texto);
            QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
            proxy->setFilterRegularExpression(re);
            });

        if (sel > -1) {
            QModelIndex index = ui->listViewTables->model()->index(sel, 0);
            ui->listViewTables->setCurrentIndex(index);
        }

        ui->statusbar->showMessage("Tables updated");
    }

    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
}


void MainWindow::refresh_log(QString selectedHost) {

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QApplication::processEvents();

    ui->statusbar->showMessage("Connecting database...");

    QSqlQuery query(QSqlDatabase::database("pref_connection"));
    ui->statusbar->showMessage("Loading tables...");
    if (query.exec("SHOW TABLES")) {
        ui->lineEditTables->setText("");

        QStandardItemModel* modelo = new QStandardItemModel(this);
        QIcon iconeTabela;
        if (ui->buttonEditTables->isChecked())
        {
            iconeTabela = QIcon(":/icons/resources/size.svg");
        }
        else {
            iconeTabela = QIcon(":/icons/resources/copy.svg");
        }

        int sel = -1;
        int cnt = 0;

        while (query.next()) {
            QApplication::processEvents();
            QString name = query.value(0).toString();

            QStandardItem* linha = new QStandardItem(iconeTabela, name);
            modelo->appendRow(linha);

            if (false) {
                sel = cnt;
            }
            cnt++;
        }

        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(modelo);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

        ui->listViewTables->setModel(proxy);

        connect(ui->lineEditTables, &QLineEdit::textChanged, this, [=](const QString& texto) {
            QString pattern = QString("(%1)").arg(texto);
            QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
            proxy->setFilterRegularExpression(re);
        });

        if (sel > -1) {
            QModelIndex index = ui->listViewTables->model()->index(sel, 0);
            ui->listViewTables->setCurrentIndex(index);
        }

        ui->statusbar->showMessage("Tables updated");
    }

    QApplication::restoreOverrideCursor();
    QApplication::processEvents();
}

void MainWindow::on_listViewConns_clicked(const QModelIndex& index)
{
    ui->actionNew_schema->setDisabled(false);
    ui->actionUsers->setDisabled(false);
    ui->actionRestore->setDisabled(false);
}

void MainWindow::on_listViewConns_doubleClicked(const QModelIndex& index)
{
    if (ui->buttonEditConns->isChecked())
    {
        listViewConns_edit(index);
    }
    else {
        listViewConns_open(index);
    }
}


void MainWindow::on_listViewSchemas_clicked(const QModelIndex& index)
{
    actual_schema = index.data(Qt::DisplayRole).toString();
    ui->actionBackup->setDisabled(false);
    ui->actionStatistics->setDisabled(false);
    ui->actionNew_table->setDisabled(false);


    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + actual_host);

    QSqlQuery query(db);

    QString consulta = "SELECT table_schema AS name, ROUND(SUM(data_length + index_length) / 1024 / 1024, 2) size "
        "FROM information_schema.TABLES "
        "WHERE table_schema = '" + actual_schema + "' "
        "GROUP BY table_schema ";
    if (query.exec(consulta)) {
        if (query.next()) {
            ui->labelSchemaSize->setText(query.value("size").toString() + " Mb");
        }
    }

    consulta = "SELECT COUNT(*) AS total_tables FROM information_schema.tables "
        "WHERE table_schema = '" + actual_schema + "'";
    if (query.exec(consulta)) {
        if (query.next()) {
            ui->labelSchemaTables->setText(query.value("total_tables").toString());
        }
    }
}


void MainWindow::on_listViewSchemas_doubleClicked(const QModelIndex& index)
{
    actual_schema = index.data(Qt::DisplayRole).toString();
    ui->actionNew_table->setDisabled(false);
    refresh_schema(actual_schema);
}


void MainWindow::on_listViewTables_clicked(const QModelIndex& index)
{
    actual_table = index.data(Qt::DisplayRole).toString();

    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + actual_host);

    QSqlQuery query(db);

    QString consulta = "SELECT table_name AS 'table', table_rows AS 'rows',"
        "ROUND((data_length + index_length) / 1024 / 1024, 2) AS 'size'"
        "FROM  information_schema.tables WHERE  table_schema = '" + actual_schema + "' AND "
        " table_name = '" + actual_table + "'";
    if (query.exec(consulta)) {
        if (query.next()) {
            ui->labelTablesSize->setText(query.value("size").toString() + " Mb");
            ui->labelTablesRows->setText(query.value("rows").toString());
        }
    }
}


void MainWindow::on_listViewTables_doubleClicked(const QModelIndex& index)
{
    // actual_table= index.data(Qt::DisplayRole).toString();
    if (ui->buttonEditTables->isChecked())
    {
        handleListViewTables_edit(index);
    }
    else {
        handleListViewTables_open(index);
    }
}

void MainWindow::open_selected_favorite(const QModelIndex& index, const bool& run)
{
    QString a_host = "";
    QString a_schema = "";
    QString a_table = "";
    QString a_color = "";

    QString name = "";
    QString value = "";
    QString selFav = index.data(Qt::DisplayRole).toString();

    int i = 0;
    for (const QString favRec : favName) {
        QList favList = favRec.split("^");
        QString fav = favList[5];
        if (fav == selFav)
        {

            // QString fav = favName[index.row()];
            // QStringList favList = fav.split("^");

            QMdiSubWindow* prev = ui->mdiArea->activeSubWindow();
            bool maximize = true;

            if (prev) {
                if (!prev->isMaximized()) {
                    maximize = false;
                }
            }

            a_host = favList[1];
            a_schema = favList[2];
            a_table = favList[3];
            a_color = favList[4];

            Sql* form = new Sql(a_host, a_schema, a_table, a_color, favName[i], favValue[i], run);

            QMdiSubWindow* sub = new QMdiSubWindow;
            sub->setWidget(form);
            sub->setAttribute(Qt::WA_DeleteOnClose);
            ui->mdiArea->addSubWindow(sub);
            sub->resize(500, 360);
            if (maximize)
                sub->showMaximized();
            else
                sub->show();
        }
        ++i;
    }
}

void MainWindow::on_treeViewFavorites_doubleClicked(const QModelIndex& index)
{
    bool run = true;
    if (ui->buttonEditFavorites->isChecked())
    {
        run = false;
    }
    open_selected_favorite(index, run);
}

void MainWindow::on_treeViewFavorites_clicked(const QModelIndex& index)
{

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

void MainWindow::on_buttonEditFavorites_clicked()
{
    refresh_favorites();
}


void MainWindow::on_buttonUpdateSchemas_clicked()
{
    refresh_schemas(actual_host, false);
}


void MainWindow::on_buttonUpdateTables_clicked()
{
    refresh_tables(actual_host);
}

void MainWindow::on_buttonUpdateFavorites_clicked()
{
    refresh_favorites();
}


void MainWindow::on_buttonNewConns_clicked()
{
    on_actionNew_connection_triggered();
}

void MainWindow::on_buttonFilterTables_clicked()
{
    QString pref_name = "fav_" + actual_host + "_" + actual_schema;

    if (ui->buttonFilterTables->isChecked())
    {
        if (ui->lineEditTables->text() == "")
        {
            QString value = getStringPreference(pref_name);
            if (value != "")
            {
                ui->lineEditTables->setText(value);
            }
        }
        else {
            setStringPreference(pref_name, ui->lineEditTables->text());
        }
    }
    else {
        ui->lineEditTables->setText("");
    }
}


void MainWindow::on_buttonFilterSchemas_clicked()
{
    QString pref_name = "fav_" + actual_host;

    if (ui->buttonFilterSchemas->isChecked())
    {
        if (ui->lineEditSchemas->text() == "")
        {
            QString value = getStringPreference(pref_name);
            if (value != "")
            {
                ui->lineEditSchemas->setText(value);
            }
        }
        else {
            setStringPreference(pref_name, ui->lineEditSchemas->text());
        }
    }
    else {
        ui->lineEditSchemas->setText("");
    }
}

void MainWindow::on_buttonFilterFavorites_clicked()
{
    QString pref_name = "fav_fav";

    if (ui->buttonFilterFavorites->isChecked())
    {
        if (ui->lineEditFavorites->text() == "")
        {
            QString value = getStringPreference(pref_name);
            if (value != "")
            {
                ui->lineEditFavorites->setText(value);
            }
        }
        else {
            setStringPreference(pref_name, ui->lineEditFavorites->text());
        }
    }
    else {
        ui->lineEditFavorites->setText("");
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
    QString selected;
    bool fez = false;
    newConnectionCount = 0;
    while (!fez)
    {
        selected = "New connection";
        if (newConnectionCount > 0) {
            selected += " " + std::to_string(newConnectionCount);
        }
        newConnectionCount++;
        fez = addConnection(selected);
        if (fez)
        {
            Connection* janela = new Connection(selected, this);
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
    }
    else {
        currentTheme = "light";
    }
    changeTheme();
    refresh_connections();
}




void MainWindow::listViewConns_open(const QModelIndex& index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    if (host_connect(actual_host))
    {
        ui->buttonFilterSchemas->setChecked(false);
        ui->buttonFilterTables->setChecked(false);
        ui->toolBoxLeft->setCurrentIndex(3);
        QJsonObject item = getConnection(actual_host);
        actual_color = item["color"].toString();
    }
}

void MainWindow::listViewConns_edit(const QModelIndex& index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    Connection* janela = new Connection(actual_host, this);
    int result = janela->exec();
    if (result == QDialog::Accepted) {
        refresh_connections();
    }
}

void MainWindow::listViewConns_clone(const QModelIndex& index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    QJsonObject orig = getConnection(actual_host);
    QString selected;
    bool fez = false;
    newConnectionCount = 0;
    while (!fez)
    {
        selected = actual_host;
        if (newConnectionCount > 0) {
            selected += " " + std::to_string(newConnectionCount);
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
            orig["ssh_keyfile"].toVariant().toString());
        if (fez)
        {
            Connection* janela = new Connection(selected, this);
            int result = janela->exec();
            if (result == QDialog::Accepted) {
                refresh_connections();
            }
        }
    }
}

void MainWindow::listViewConns_remove(const QModelIndex& index)
{
    actual_host = index.data(Qt::DisplayRole).toString();
    deleteConnection(actual_host);
    refresh_connections();
}

void MainWindow::show_context_menu_Conns(const QPoint& pos)
{
    QModelIndex index = ui->listViewConns->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction* schemaOpen = menu.addAction("Open");
    QAction* schemaEdit = menu.addAction("Edit");
    QAction* schemaClone = menu.addAction("Clone");
    QAction* schemaRemove = menu.addAction("Remove");
    QAction* schemaBatchRun = menu.addAction("Batch run");

    QAction* selectedAction = menu.exec(ui->listViewConns->viewport()->mapToGlobal(pos));

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


void MainWindow::show_context_menu_Schemas(const QPoint& pos)
{
    QModelIndex index = ui->listViewSchemas->indexAt(pos);
    QString selSchema = index.data(Qt::DisplayRole).toString();
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction* schemaOpen = menu.addAction("Open");
    QAction* schemaCreate = menu.addAction("Create");
    QAction* schemaDrop = menu.addAction("Drop");
    QAction* schemaRefresh = menu.addAction("Refresh");
    menu.addSeparator();
    QAction* schemaUsers = menu.addAction("Users");
    QAction* schemaStatistics = menu.addAction("Statistics");
    menu.addSeparator();
    QAction* schemaBackup = menu.addAction("Backup");
    QAction* schemaRestore = menu.addAction("Restore");
    QAction* schemaBatchRun = menu.addAction("Batch run");

    QAction* selectedAction = menu.exec(ui->listViewSchemas->viewport()->mapToGlobal(pos));

    if (selectedAction == schemaOpen) {
        on_listViewSchemas_clicked(index);
    }
    else if (selectedAction == schemaStatistics) {
        Statistics* janela = new Statistics(actual_host, selSchema, this);
        janela->exec();
    }
    else if (selectedAction == schemaBackup) {
        backup(actual_host, selSchema, this);
    }
    else if (selectedAction == schemaRestore)
    {
        restore(actual_host, selSchema, this);
    }
    else if (selectedAction == schemaBatchRun) {
        batch_run();
    }
    else if (selectedAction == schemaRefresh) {
        refresh_schemas(actual_host, false);
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
            "    min-width: 90px;"
            "    min-height: 20px;"
            "}"
        );
        msgBox.setWindowTitle("Confirm");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);

        QString message = QString("Are you sure you want to drop database\n\"%1\"?").arg(dbName);
        msgBox.setText(message);

        QMessageBox::StandardButton reply = static_cast<QMessageBox::StandardButton>(msgBox.exec());

        if (reply == QMessageBox::Yes) {
            QSqlQuery query(QSqlDatabase::database("mysql_connection_" + actual_host));
            QString sql = QString("DROP DATABASE `%1`;").arg(dbName);

            if (!query.exec(sql)) {
                customAlert("Error", "Failed to delete database:\n" + query.lastError().text());
            }
            else {
                refresh_schemas(actual_host, false);
                // customAlert("Success", "Database deleted successfully.");

            }
        }
    }
}

void MainWindow::show_context_menu_Tables(const QPoint& pos)
{
    QModelIndex index = ui->listViewTables->indexAt(pos);
    if (!index.isValid())
        return;

    QMenu menu(this);
    QAction* tableOpen = menu.addAction("Open");
    QAction* tableEdit = menu.addAction("Edit");
    QAction* tableCreate = menu.addAction("Create");
    QAction* tableDrop = menu.addAction("Drop");
    QAction* tableRefresh = menu.addAction("Refresh");
    menu.addSeparator();
    QAction* tableCopySQL = menu.addAction("Copy as SQL");
    QAction* tableCopyCSV = menu.addAction("Copy as CSV");

    QAction* selectedAction = menu.exec(ui->listViewTables->viewport()->mapToGlobal(pos));

    if (selectedAction == tableOpen) {
        handleListViewTables_open(index);
    }
    else if (selectedAction == tableCopySQL) {
        QString tableName = index.data(Qt::DisplayRole).toString();
        QString connectionName = "mysql_connection_" + actual_host;

        QString createTableSql = generateCreateTableStatement(tableName, connectionName);

        if (!createTableSql.isEmpty()) {

            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(createTableSql);

            QSqlQuery query(QSqlDatabase::database(connectionName));
            if (query.exec(createTableSql)) {
                // qDebug() << "Tabela criada com sucesso";
            }
            else {
                // qDebug() << "Falha ao criar tabela:" << query.lastError().text();
            }
            // } else {
            //     qDebug() << "Falha ao gerar o comando CREATE TABLE";
        }
    }
    else if (selectedAction == tableCopyCSV) {
        QString tableName = index.data(Qt::DisplayRole).toString();
        QString connectionName = "mysql_connection_" + actual_host;

        QString csvContent = generateColumnsCsv(tableName, connectionName);

        if (!csvContent.isEmpty()) {
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(csvContent);
            // qDebug() << "Campos da tabela copiados para a área de transferência em formato CSV:\n" << csvContent;
        // } else {
        //     qDebug() << "Falha ao gerar o CSV dos campos da tabela";
        }

    }
    else if (selectedAction == tableRefresh) {
        refresh_tables(actual_host);
    }
    else if (selectedAction == tableEdit) {
        handleListViewTables_edit(index);
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
            "    min-height: 20px;"
            "}"
        );
        msgBox.setWindowTitle("Confirm");
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
            }
            else {
                refresh_tables(actual_host);
                // customAlert("Success", "Table deleted successfully.");

            }
        }
    }

}


void MainWindow::show_context_menu_Favorites(const QPoint& pos)
{
    QModelIndex index = ui->treeViewFavorites->indexAt(pos);
    if (!index.isValid())
        return;
    QString selectedFavoriteName = index.data(Qt::DisplayRole).toString();
    QModelIndex parentIndex = index.parent();
    QString parentName = "Local";
    if (parentIndex.isValid()) {
        parentName = parentIndex.data(Qt::DisplayRole).toString();
        qDebug() << "O item pertence ao grupo:" << parentName;
    }
    else {
        return;
    }
    qDebug() << selectedFavoriteName << "Parent:" << parentName;
    int i = 0;
    int favIndex = 0;

    for (const QString favRec : favName) {
        QList favList = favRec.split("^");
        QString fav = favList[5];
        if (fav == selectedFavoriteName)
        {
            favIndex = i;
        }
        i++;
    }

    QMenu menu(this);
    QAction* favoritesOpen = menu.addAction("Open");
    QAction* favoritesEdit = menu.addAction("Edit");
    QAction* favoritesClone = menu.addAction("Clone");
    QAction* favoritesRename = menu.addAction("Rename");
    QAction* favoritesDelete = menu.addAction("Delete");
    menu.addSeparator();
    QAction* favoritesRefresh = menu.addAction("Refresh");
    QAction* selectedAction = menu.exec(ui->treeViewFavorites->viewport()->mapToGlobal(pos));

    if (selectedAction == favoritesOpen) {
        open_selected_favorite(index, true);
    }
    else if (selectedAction == favoritesEdit) {
        open_selected_favorite(index, false);
    }
    else if (selectedAction == favoritesClone) {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Clone favorite"));

        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        QLabel* label = new QLabel(tr("Name:"), &dialog);
        layout->addWidget(label);

        QLineEdit* lineEdit = new QLineEdit(&dialog);
        lineEdit->setText(selectedFavoriteName);
        layout->addWidget(lineEdit);

        lineEdit->setStyleSheet(
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

        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttons);

        QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            QString name = lineEdit->text().trimmed();
            if (!name.isEmpty() && name != selectedFavoriteName) {
                QString value = getStringPreference(favName[index.row()]);
                QString newFavName = favName[favIndex].replace(selectedFavoriteName, name);
                if (parentName == "Local")
                {
                    setStringPreference(newFavName, value);
                }
                else {
                    setStringSharedPreference(newFavName, value);
                }

                // QSqlQuery query(QSqlDatabase::database("pref_connection"));

                refresh_favorites();
            }
        }
    }
    else if (selectedAction == favoritesRename) {
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Rename favorite"));

        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        QLabel* label = new QLabel(tr("Name:"), &dialog);
        layout->addWidget(label);

        QLineEdit* lineEdit = new QLineEdit(&dialog);
        lineEdit->setText(selectedFavoriteName);
        layout->addWidget(lineEdit);

        lineEdit->setStyleSheet(
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

        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttons);

        QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            QString name = lineEdit->text().trimmed();
            if (!name.isEmpty() && name != selectedFavoriteName) {


                if (parentName == "Local")
                {
                    QString value = getStringPreference(favName[favIndex]);
                    QString newFavName = favName[favIndex];
                    newFavName.replace(selectedFavoriteName, name);

                    // qDebug() << "nome antigo: " << favName[favIndex];
                    // qDebug() << "nome novo: " << newFavName;
                    setStringPreference(newFavName, value);


                    QSqlQuery query(QSqlDatabase::database("pref_connection"));

                    query.prepare("DELETE FROM prefs WHERE name = :name");
                    query.bindValue(":name", favName[favIndex]);
                    if (!query.exec()) {
                        qCritical() << "Erro ao excluir favorito:" << query.lastError().text();
                        return;
                    }

                }
                else {
                    QString value = getStringSharedPreference(favName[favIndex]);
                    QString newFavName = favName[favIndex];
                    newFavName.replace(selectedFavoriteName, name);

                    setStringSharedPreference(newFavName, value);


                    QSqlQuery query(QSqlDatabase::database("mysql_connection_" + sharedFavoriteDB));

                    query.prepare("DELETE FROM _SequelFast.prefs WHERE name = :name");
                    query.bindValue(":name", favName[favIndex]);
                    if (!query.exec()) {
                        qCritical() << "Erro ao excluir favorito:" << query.lastError().text();
                        return;
                    }

                }

                refresh_favorites();
            }
        }
    }
    else if (selectedAction == favoritesDelete) {

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

        QString message = QString("Are you sure you want to delete favorite\n\"%1\"?").arg(selectedFavoriteName);
        msgBox.setText(message);

        QMessageBox::StandardButton reply = static_cast<QMessageBox::StandardButton>(msgBox.exec());

        if (reply == QMessageBox::Yes) {
            if (parentName == "Local")
            {
                QSqlQuery query(QSqlDatabase::database("pref_connection"));

                query.prepare("DELETE FROM prefs WHERE name = :name");
                query.bindValue(":name", favName[favIndex]);

                if (!query.exec()) {
                    qCritical() << "Erro ao excluir favorito:" << query.lastError().text();
                    return;
                }

            }
            else {
                QSqlQuery query(QSqlDatabase::database("mysql_connection_" + sharedFavoriteDB));

                query.prepare("DELETE FROM _SequelFast.prefs WHERE name = :name");
                query.bindValue(":name", favName[favIndex]);

                if (!query.exec()) {
                    qCritical() << "Erro ao excluir favorito:" << query.lastError().text();
                    return;
                }

            }
            refresh_favorites();
        }
    }
    else if (selectedAction == favoritesRefresh) {
        refresh_favorites();
    }
}



void MainWindow::handleListViewTables_open(const QModelIndex& index)
{
    actual_table = index.data(Qt::DisplayRole).toString();
    QMdiSubWindow* prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }

    Sql* form = new Sql(actual_host, actual_schema, actual_table, actual_color, "", "", true);

    QMdiSubWindow* sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);
    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();

}

void MainWindow::handleListViewTables_edit(const QModelIndex& index)
{
    actual_table = index.data(Qt::DisplayRole).toString();
    QMdiSubWindow* prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }
    Structure* form = new Structure(actual_host, actual_schema, actual_table);

    QMdiSubWindow* sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);

    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();
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
    QMdiSubWindow* prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }

    Users* form = new Users(actual_host, actual_schema, this);

    QMdiSubWindow* sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);
    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();

}

void MainWindow::batch_run()
{
    QMdiSubWindow* prev = ui->mdiArea->activeSubWindow();
    bool maximize = true;

    if (prev) {
        if (!prev->isMaximized()) {
            maximize = false;
        }
    }

    Batch* form = new Batch(actual_host, actual_schema, this);

    QMdiSubWindow* sub = new QMdiSubWindow;
    sub->setWidget(form);
    sub->setAttribute(Qt::WA_DeleteOnClose);
    ui->mdiArea->addSubWindow(sub);
    sub->resize(500, 360);
    if (maximize)
        sub->showMaximized();
    else
        sub->show();
}

void MainWindow::backup(const QString& bkp_host, const QString& bkp_schema, QWidget* parent)
{
    Backup dialog(bkp_host, bkp_schema, this);
    dialog.exec();
    refresh_schemas(actual_host, false);
}

void MainWindow::restore(const QString& bkp_host, const QString& bkp_schema, QWidget* parent)
{
    Restore executor;
    executor.run("", "mysql_connection_", bkp_host, bkp_schema, this);
    refresh_schemas(actual_host, false);
}

void MainWindow::on_actionBackup_triggered()
{
    backup(actual_host, actual_schema, this);
}


void MainWindow::on_actionRestore_triggered()
{
    restore(actual_host, actual_schema, this);
}


void MainWindow::on_actionStatistics_triggered()
{
    Statistics* janela = new Statistics(actual_host, actual_schema, this);
    janela->exec();
}

void MainWindow::log(QString host, QString schema, QString str)
{
    QString agora = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    int row = modelLog->rowCount();
    modelLog->insertRow(row);

    modelLog->setData(modelLog->index(row, 0), agora);
    modelLog->setData(modelLog->index(row, 1), host);
    modelLog->setData(modelLog->index(row, 2), schema);
    modelLog->setData(modelLog->index(row, 3), str);

    QModelIndex lastIdx = modelLog->index(row, 0); // guarda o último índice

    // Seleciona e exibe a última linha
    ui->tableLogView->resizeColumnsToContents();
    if (lastIdx.isValid()) {
        ui->tableLogView->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tableLogView->selectRow(lastIdx.row());                   // seleciona a linha inteira
        ui->tableLogView->setCurrentIndex(lastIdx);                   // move o cursor
        ui->tableLogView->scrollTo(lastIdx, QAbstractItemView::PositionAtBottom); // rola até ela
        ui->tableLogView->setFocus();
    }
    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    QString insertSql = "INSERT INTO logs (date, host, schema, query) VALUES (:date, :host, :schema, :query)";
    query.prepare(insertSql); // Prepara a consulta para inserção segura (evita SQL injection)

    query.bindValue(":date", agora);
    query.bindValue(":host", host);
    query.bindValue(":schema", schema);
    query.bindValue(":query", str);

    if (!query.exec()) {
        qWarning() << "Erro ao inserir host:" << query.lastError().text();
    }

}

void MainWindow::on_buttonDeleteLog_clicked()
{
    qDebug() << "Delete log";
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
        "    min-width: 90px;"
        "    min-height: 20px;"
        "}"
        );
    msgBox.setWindowTitle("Confirm");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    QString message = QString("Are you sure you want to clear log?");
    msgBox.setText(message);

    QMessageBox::StandardButton reply = static_cast<QMessageBox::StandardButton>(msgBox.exec());

    if (reply == QMessageBox::Yes) {
        modelLog->clear();

        QSqlQuery query(QSqlDatabase::database("pref_connection"));

        QString deleteSql = "DELETE FROM logs";
        query.prepare(deleteSql); // Prepara a consulta para inserção segura (evita SQL injection)

        if (!query.exec()) {
            qWarning() << "Erro ao excluir log:" << query.lastError().text();
        }
    }

}

