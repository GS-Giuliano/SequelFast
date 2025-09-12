#include "backup.h"
#include "two_checkbox_delegate.h"
#include "functions.h"
#include "mainwindow.h"
#include "restore.h";


extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;

Backup::Backup(const QString& host, const QString& schema, QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Select tables and file name");
    bkp_host = host;
    bkp_schema = schema;

    QString timestamp = QDateTime::currentDateTime().toString("yyMMdd-HHmmss");
    QString dumpFile = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
        + "/" + schema + "-" + timestamp + ".sql";

    auto* layout = new QVBoxLayout(this);

    // Linha 1: campo de arquivo + botão
    auto* fileLabel = new QLabel("To file");
    fileLabel->setFixedWidth(150);
    auto* fileLayout = new QHBoxLayout;
    lineEdit = new QLineEdit(this);
    lineEdit->setText(dumpFile);


    btnBrowse = new QPushButton("Select...", this);
    fileLayout->addWidget(fileLabel);
    fileLayout->addWidget(lineEdit);
    fileLayout->addWidget(btnBrowse);
    layout->addLayout(fileLayout);
    connect(btnBrowse, &QPushButton::clicked, this, &Backup::chooseFile);

    // Linha 2: seleção de schema
    auto* schemaLayout = new QHBoxLayout;
    auto* schemaLabel = new QLabel("To schema");
    schemaLabel->setFixedWidth(150);
    schemaEdit = new QLineEdit(bkp_schema);
    connect(schemaEdit, &QLineEdit::textChanged, this, &Backup::onSchemaEditTextChanged);

    schemaLayout->addWidget(schemaLabel);
    schemaLayout->addWidget(schemaEdit);
    layout->addLayout(schemaLayout);

    // Linha 3: seleção de host
    auto* connLayout = new QHBoxLayout;
    auto* connLabel = new QLabel("To connection");
    connLabel->setFixedWidth(150);
    connList = new QComboBox();
    connLayout->addWidget(connLabel);
    connLayout->addWidget(connList);
    layout->addLayout(connLayout);

    // Linha 4: seleção de tabelas
    tableView = new QTableView(this);
    layout->addWidget(tableView);

    connect(tableView->horizontalHeader(), &QHeaderView::sectionClicked, this, &Backup::onHeaderClicked);

    this->setStyleSheet(
        "QLabel:last {"
        " padding-right: 10px;"
        " min-height: 30px;"
        " min-width: 280px;"
        "}"
        "QPushButton {"
        " padding: 4px;"
        " min-height: 16px;"
        " min-width: 90px;"
        "}"
    );

    // Linha 5: botões
    auto* buttonLayout = new QHBoxLayout;
    btnFavorite = new QPushButton("Favorite", this);
    btnCancel = new QPushButton("Cancel", this);
    btnConfirm = new QPushButton("Run", this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnFavorite);
    buttonLayout->addWidget(btnCancel);
    buttonLayout->addWidget(btnConfirm);
    layout->addLayout(buttonLayout);

    connect(btnFavorite, &QPushButton::clicked, this, &Backup::onFavorite);
    connect(btnCancel, &QPushButton::clicked, this, &Backup::onCancel);
    connect(btnConfirm, &QPushButton::clicked, this, &Backup::onConfirm);

    // Progresso de tabelas
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    progressBar->setFormat("%v / %m");
    layout->addWidget(progressBar);

    // Progresso de registros
    progressBar2 = new QProgressBar(this);
    progressBar2->setRange(0, 100);
    progressBar2->setValue(0);
    progressBar2->setVisible(false);
    progressBar2->setFormat("%v / %m");
    layout->addWidget(progressBar2);

    refresh_tables();
    refresh_conns();

    setMinimumSize(700, 500);
}

void Backup::refresh_conns()
{
    connList->clear();
    connList->addItem("Select host destination...");
    for (const QJsonValue& valor : connections) {
        if (valor.isObject()) {
            QJsonObject item = valor.toObject();
            QString nome = item["name"].toString();
            // QString corDeFundo = item["color"].toString().toLower();
            connList->addItem(nome);
        }
    }
}
void Backup::refresh_tables()
{
    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + bkp_host);
    QSqlQuery useQuery(db);
    QSqlQuery tablesQuery(db);

    // Executa "USE schema"
    if (!useQuery.exec("USE " + bkp_schema)) {
        // qDebug() << "Erro ao selecionar schema:" << useQuery.lastError().text();
        return;
    }

    // Executa "SHOW TABLES"
    if (!tablesQuery.exec("SHOW TABLES")) {
        // qDebug() << "Erro ao buscar tabelas:" << tablesQuery.lastError().text();
        return;
    }

    model = new QStandardItemModel(0, 5, this);
    model->setHorizontalHeaderLabels({ "Table", "Structure", "Data", "Where", "Order by", "Limit" });
    tableView->setModel(model);

    checkboxDelegate = new TwoCheckboxDelegate(this);
    tableView->setItemDelegateForColumn(1, checkboxDelegate);
    tableView->setItemDelegateForColumn(2, checkboxDelegate);

    int cnt = 0;
    QString where;
    QString orderBy;
    QString limit;
    QString favPrefix = "bkp-table:";
    while (tablesQuery.next()) {
        int row = model->rowCount();
        QString name = tablesQuery.value(0).toString();
        bool structure = getStringPreference(favPrefix + name + ":structure") == "0" ? false : true;
        bool tableData = getStringPreference(favPrefix + name + ":tableData") == "0" ? false : true;
        where = getStringPreference(favPrefix + name + ":where");
        orderBy = getStringPreference(favPrefix + name + ":orderBy");
        limit = getStringPreference(favPrefix + name + ":limit");

        model->insertRow(row);
        model->setData(model->index(row, 0), name);
        model->setData(model->index(row, 1), structure, Qt::UserRole);
        model->setData(model->index(row, 2), tableData, Qt::UserRole);
        model->setData(model->index(row, 3), where, Qt::EditRole);
        model->setData(model->index(row, 4), orderBy, Qt::EditRole);
        model->setData(model->index(row, 5), limit, Qt::EditRole);
        cnt++;
    }
    tableView->resizeColumnsToContents();
}


void Backup::chooseFile()
{
    bool pathValid = false;
    QString file;

    while (!pathValid) {
        file = QFileDialog::getSaveFileName(this, "Select file path and name", "", "SQL Files (*.sql);;All Files (*)");

        if (file.isEmpty()) {
            // Usuário cancelou a seleção
            lineEdit->setText(dumpFile);
            return;
        }

        QFileInfo fileInfo(file);

        if (fileInfo.exists()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "File Exists",
                QString("The file '%1' already exists. Do you want to overwrite it?").arg(fileInfo.fileName()),
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                // Usuário aceitou sobrescrever
                lineEdit->setText(file);
                pathValid = true;
            }
            else {
                // Usuário não quer sobrescrever, solicita nova seleção
                continue;
            }
        }
        else {
            // Verifica se o diretório existe
            QDir dir(fileInfo.absolutePath());
            if (!dir.exists()) {
                QMessageBox::warning(
                    this,
                    "Invalid Directory",
                    QString("The directory '%1' does not exist. Please select a valid directory.").arg(fileInfo.absolutePath())
                );
                // Solicita nova seleção
                continue;
            }
            // Arquivo não existe e diretório é válido
            lineEdit->setText(file);
            pathValid = true;
        }
    }
}

void Backup::closeEvent(QCloseEvent* event)
{
    onCancel();
    event->ignore();
}

void Backup::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        onCancel();
        event->accept();
    }
    else {
        QDialog::keyPressEvent(event);
    }
}

void Backup::onFavorite()
{
    QString favPrefix = "bkp-table:";
    for (int row = 0; row < model->rowCount(); ++row) {
        QString table = model->data(model->index(row, 0), Qt::DisplayRole).toString();
        bool structure = model->data(model->index(row, 1), Qt::UserRole).toBool();
        bool tableData = model->data(model->index(row, 2), Qt::UserRole).toBool();
        QString where = model->data(model->index(row, 3), Qt::EditRole).toString();
        QString orderBy = model->data(model->index(row, 4), Qt::EditRole).toString();
        QString limit = model->data(model->index(row, 5), Qt::EditRole).toString();
        setStringPreference(favPrefix + table + ":structure", (structure ? "1" : "0"));
        setStringPreference(favPrefix + table + ":tableData", (tableData ? "1" : "0"));
        setStringPreference(favPrefix + table + ":where", where);
        setStringPreference(favPrefix + table + ":orderBy", orderBy);
        setStringPreference(favPrefix + table + ":limit", limit);
    }
}

void Backup::onCancel()
{
    abort = true;
    if (!running)
        reject();
}

void Backup::onConfirm()
{
    running = true;
    bool db2Error = false;
    bool exportToFile = false;
    bool exportToHost = false;
    QString exportToHostName = bkp_host;
    QString exportToSchema = schemaEdit->text().trimmed();
    QFile file(lineEdit->text());
    exportToHostName = bkp_host;
    int totalTables = model->rowCount() - 1; // pula linha 0
    int processedTables = 0;

    if (lineEdit->text() != "")
    {
        exportToFile = true;
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Failed to open file for writing: " + file.errorString());
            qDebug() << "Error opening file:" << file.errorString();
            return;
        }
    }

    if (connList->currentText() != "Select host destination...")
    {
        exportToHost = true;
        exportToHostName = connList->currentText();
    }

    QTextStream out(&file);

    if (exportToFile)
    {
        out << "CREATE DATABASE IF NOT EXISTS " << exportToSchema << ";\n";
        out << "USE `" << exportToSchema << "`;\n\n";
    }

    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + bkp_host);
    QSqlQuery query(db);

    progressBar->setVisible(true);
    progressBar->setValue(0);
    progressBar->setRange(0, totalTables);
    QApplication::processEvents();

    out << "SET FOREIGN_KEY_CHECKS=0;\n";
    out << "SET UNIQUE_CHECKS=0;\n";
    out << "SET autocommit=0;\n";
    out << "START TRANSACTION;\n";


    for (int row = 0; row < model->rowCount(); ++row) {
        if (!abort)
        {
            QString table = model->data(model->index(row, 0), Qt::DisplayRole).toString();
            QString where = model->data(model->index(row, 3), Qt::DisplayRole).toString();
            QString orderBy = model->data(model->index(row, 4), Qt::DisplayRole).toString();
            QString limit = model->data(model->index(row, 5), Qt::DisplayRole).toString();

            bool structure = model->data(model->index(row, 1), Qt::UserRole).toBool();
            bool data = model->data(model->index(row, 2), Qt::UserRole).toBool();

            // qDebug() << "Processing table:" << table << "Structure:" << structure << "Data:" << data;

            processedTables++;
            progressBar->setValue(processedTables);
            QApplication::processEvents();


            if ((structure || data) && (exportToFile)) {
                // Obtém a estrutura da tabela
                if (structure) {
                    if (query.exec("SHOW CREATE TABLE " + table)) {
                        if (query.next()) {
                            QString createTableStmt = query.value(1).toString();
                            if (exportToFile) {
                                out << "DROP TABLE IF EXISTS " << table << ";\n";
                                out << createTableStmt << ";\n\n";
                            }
                        }
                        else {
                            progressBar->setVisible(false);
                            progressBar2->setVisible(false);
                            qDebug() << "Error: No result for SHOW CREATE TABLE" << table;
                            if (exportToFile)
                            {
                                out << "-- Error: Could not retrieve structure for table " << table << "\n\n";
                            }
                        }
                    }
                    else {
                        progressBar->setVisible(false);
                        progressBar2->setVisible(false);
                        qDebug() << "Error executing SHOW CREATE TABLE for" << table << ":" << query.lastError().text();
                        if (exportToFile)
                        {
                            out << "-- Error: Could not retrieve structure for table " << table << "\n\n";
                        }

                    }
                }

                // Obtém os dados da tabela
                if (data) {
                    progressBar2->setVisible(true);

                    int totalRows;
                    int processedRows = 0;
                    QString suffix = "";

                    if (where != "")
                    {
                        suffix = suffix + " WHERE " + where;
                    }
                    if (orderBy != "")
                    {
                        suffix = suffix + " ORDER BY " + orderBy;
                    }
                    if (limit != "")
                    {
                        suffix = suffix + " LIMIT " + limit;
                    }

                    if (query.exec("SELECT COUNT(*) FROM " + bkp_schema + "." + table + suffix) && query.next()) {
                        totalRows = query.value(0).toInt();
                    }
                    progressBar2->setVisible(true);
                    progressBar2->setRange(0, totalRows);


                    if (query.exec("SELECT * FROM " + bkp_schema + "." + table + suffix)) {

                        QSqlRecord record = query.record();
                        int columnCount = record.count();
                        QStringList columnNames;
                        for (int i = 0; i < columnCount; ++i) {
                            columnNames << "`" + record.fieldName(i) + "`" ;
                        }
                        while (query.next()) {
                            if (!abort)
                            {
                                processedRows++;
                                progressBar2->setValue(processedRows);
                                progressBar2->setFormat("%v / " + table + " / %m");

                                QApplication::processEvents();

                                QStringList values;
                                for (int i = 0; i < columnCount; ++i) {
                                    QVariant value = query.value(i);
                                    if (value.isNull()) {
                                        values << "NULL";
                                    }
                                    else if (value.type() == QVariant::String) {
                                        QString safeStr = value.toString();
                                        safeStr.replace("'", "''");     // escapa aspas simples
                                        safeStr.replace("\r", "\\r");   // escapa CR
                                        safeStr.replace("\n", "\\n");   // escapa LF
                                        values << "'" + safeStr + "'";
                                    }
                                    else if (value.type() == QVariant::Date || value.type() == QVariant::DateTime) {
                                        values << "'" + value.toDateTime().toString("yyyy-MM-dd HH:mm:ss") + "'";
                                    }
                                    else if (value.type() == QVariant::Time) {
                                        values << "'" + value.toTime().toString("HH:mm:ss") + "'";
                                    }
                                    else if (value.type() == QVariant::ByteArray) {
                                        QByteArray bytes = value.toByteArray();
                                        values << "X'" + QString(bytes.toHex()) + "'";
                                    }
                                    else if (value.type() == QVariant::LongLong || value.type() == QVariant::Double || value.type() == QVariant::Int) {
                                        values << value.toString();
                                    }
                                    else {
                                        values << "'" + value.toString() + "'";
                                    }
                                }
                                if (exportToFile)
                                {
                                    out << "INSERT INTO " << table << " (" << columnNames.join(", ") << ") VALUES ("
                                        << values.join(", ") << ");\n";
                                }
                            }
                        }

                        progressBar2->setVisible(false);
                        if (exportToFile)
                        {
                            out << "\n";
                        }
                    }
                    else {
                        progressBar->setVisible(false);
                        progressBar2->setVisible(false);

                        // qDebug() << "Error executing SELECT * FROM" << table << ":" << query.lastError().text();
                        if (exportToFile)
                        {
                            out << "-- Error: Could not retrieve data for table " << table << "\n\n";
                        }
                    }
                    progressBar2->setVisible(false);
                }
            }
        }
    }
    out << "COMMIT;\n";
    out << "SET autocommit=1;\n";
    out << "SET FOREIGN_KEY_CHECKS=1;\n";
    out << "SET UNIQUE_CHECKS=1;\n";

    progressBar->setVisible(false);
    progressBar2->setVisible(false);
    file.close();

    qDebug() << "exportToHostName" << exportToHostName;
    qDebug() << "exportToSchema" << exportToSchema;
    qDebug() << "lineEdit->text()" << lineEdit->text();

    if (exportToHost && lineEdit->text() != "")
    {
        qDebug() << "Restaurando!!!!";
        connectMySQL(exportToHostName, this, "backup_");
        if (!dbMysql.open())
        {
            qWarning() << "erro ao abrir DB!!!!";
        }
        Restore executor;
        executor.run(lineEdit->text(), "backup_", exportToHostName, exportToSchema, this);
    }


    // MainWindow *mainWin = qobject_cast<MainWindow *>(this->window());
    // if (mainWin) {
    //     mainWin->refresh_schemas(bkp_host, false);
    // } else {
    //     qWarning() << "MainWindow not found from Backup::refresh_schemas";
    // }


    running = false;
    if (abort)
    {
        QMessageBox::information(this, "Failed", "Backup cancelled!");
        reject();

    }
    else {
        QMessageBox::information(this, "Success", "Backup created successfully!");
        accept();
    }
}


void Backup::onHeaderClicked(int section)
{
    // Inverte os valores booleanos nas colunas 1 ("Structure") ou 2 ("Data"), a partir da linha 1 (índice 1)
    if (section == 1 || section == 2) {
        for (int row = 0; row < model->rowCount(); ++row) {
            bool currentValue = model->data(model->index(row, section), Qt::UserRole).toBool();
            model->setData(model->index(row, section), !currentValue, Qt::UserRole);
        }
    }
    if (section == 3 || section == 4 || section == 5) {
        for (int row = 0; row < model->rowCount(); ++row) {
            model->setData(model->index(row, section), "");
        }
    }
}

void Backup::onSchemaEditTextChanged(const QString& newText) {
    if (lineEdit->text() != "")
    {
        QString timestamp = QDateTime::currentDateTime().toString("yyMMdd-HHmmss");
        QString dumpFile = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
            + "/" + newText + "-" + timestamp + ".sql";
        lineEdit->setText(dumpFile);
    }
}
