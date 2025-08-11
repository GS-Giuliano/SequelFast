#include "statistics.h"
#include "ui_statistics.h"
#include <functions.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;

extern int pref_sql_limit;
extern int pref_table_row_height;
extern int pref_table_font_size;
extern int pref_sql_font_size;

Statistics::Statistics(QString& host, QString& schema, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::Statistics)
{
    QString sql_host = host;
    QString sql_schema = schema;
    // QString sql_table = table;

    ui->setupUi(this);
    this->setWindowTitle(sql_host + " • " + sql_schema);
    sql_host = host;
    sql_schema = schema;

    ui->lineName->setText(sql_schema);

    connect(ui->tableView, &QTableView::clicked, this, &Statistics::on_tableView_cellClicked);

    QString consulta = "SELECT * FROM performance_schema.global_variables";
    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + sql_host);

    QSqlQueryModel* model = new QSqlQueryModel(this);
    model->setQuery(consulta, db);

    if (model->lastError().isValid()) {
        qDebug() << model->lastError().text();
        delete model;
    }
    else {
        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(model);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

        ui->tableView->setModel(proxy);
        // ui->tableView->resizeColumnsToContents();
        ui->tableView->setColumnWidth(0, 220);
        ui->tableView->setColumnWidth(1, 130);
        ui->tableView->setSortingEnabled(true);

        connect(ui->lineFilter, &QLineEdit::textChanged, this, [=](const QString& texto) {
            QString pattern = QString("(%1)").arg(texto);
            QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
            proxy->setFilterRegularExpression(re);
            });
    }


    QSqlQuery query(db);
    if (query.exec("SELECT * FROM information_schema.SCHEMATA WHERE SCHEMA_NAME = '" + sql_schema + "'")) {
        if (query.next()) {
            ui->lineCharSet->setText(query.value("DEFAULT_CHARACTER_SET_NAME").toString());
            ui->lineCollation->setText(query.value("DEFAULT_COLLATION_NAME").toString());
            ui->lineEncrypt->setText(query.value("DEFAULT_ENCRYPTION").toString());

            consulta = "SELECT table_schema AS name, ROUND(SUM(data_length + index_length) / 1024 / 1024, 2) size "
                "FROM information_schema.TABLES "
                "WHERE table_schema = '" + sql_schema + "' "
                "GROUP BY table_schema ";
            if (query.exec(consulta)) {
                if (query.next()) {
                    ui->lineSize->setText(query.value("size").toString() + " Mb");
                }
            }

            consulta = "SELECT COUNT(*) AS total_tables FROM information_schema.tables "
                "WHERE table_schema = '" + sql_schema + "'";
            if (query.exec(consulta)) {
                if (query.next()) {
                    ui->lineTables->setText(query.value("total_tables").toString());
                }
            }

        }
    }


}

Statistics::~Statistics()
{
    delete ui;
}

void Statistics::on_tableView_cellClicked(const QModelIndex& index)
{
    if (!index.isValid())
        return;

    const QAbstractItemModel* model = index.model();


    int row = index.row();

    QString col0Name = model->headerData(0, Qt::Horizontal).toString();
    QString col1Name = model->headerData(1, Qt::Horizontal).toString();

    QString col0Value = model->index(row, 0).data(Qt::DisplayRole).toString();
    QString col1Value = model->index(row, 1).data(Qt::DisplayRole).toString();

    QString message = QString("<b>%1:</b><br>%2 <br><br>&nbsp;")
        .arg(col0Value, col1Value);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Value");
    msgBox.setText(message);
    msgBox.setDetailedText(col1Value);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setEscapeButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Information);

    QSpacerItem* horizontalSpacer = new QSpacerItem(400, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* hlayout = (QGridLayout*)msgBox.layout();
    hlayout->addItem(horizontalSpacer, hlayout->rowCount(), 0, 1, hlayout->columnCount());

    msgBox.exec();
}
