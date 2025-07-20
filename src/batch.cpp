#include "batch.h"
#include "ui_batch.h"
#include "sqlhighlighter.h"

#include <QDebug>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include <QMessageBox>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStyledItemDelegate>


#include <functions.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;
extern QString currentTheme;

Batch::Batch(QString &host, QString &schema, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Batch)
{
    ui->setupUi(this);

    new SqlHighlighter(ui->textQuery->document());

}

Batch::~Batch()
{
    delete ui;
}
