#include "batch.h"
#include "ui_batch.h"
#include "sqlhighlighter.h"
#include "functions.h"

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;
extern QString currentTheme;

Batch::Batch(QString& host, QString& schema, QWidget* parent)
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
