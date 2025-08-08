#pragma once

#include <QAbstractItemModel>
#include <QButtonGroup>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QRadioButton>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QStyledItemDelegate>


namespace Ui {
    class Batch;
}

class Batch : public QMainWindow
{
    Q_OBJECT

public:
    explicit Batch(QString& host, QString& schema, QWidget* parent);
    ~Batch();

private:
    Ui::Batch* ui;
    QString usr_host;
    QString usr_scheme;
};

