#pragma once

#include <QAbstractItemModel>
#include <QDebug>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>


namespace Ui {
    class Statistics;
}

class Statistics : public QDialog
{
    Q_OBJECT

public:
    explicit Statistics(QString& host, QString& schema, QWidget* parent = nullptr);
    ~Statistics();

private:
    Ui::Statistics* ui;

private slots:
    void on_tableView_cellClicked(const QModelIndex& index);

};


