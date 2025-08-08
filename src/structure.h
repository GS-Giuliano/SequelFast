#pragma once

#include <QCompleter>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QModelIndex>
#include <QRegularExpressionValidator>
#include <QSqlDatabase>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QStandardItemModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QVariant>

namespace Ui {
    class Structure;
}

class Structure : public QMainWindow
{
    Q_OBJECT

public:
    explicit Structure(QString& host, QString& schema, QString& table, QWidget* parent = nullptr);
    ~Structure();
    void refresh_structure();
    void log(QString name, QString what, QString value_from, QString value_to);

private:
    Ui::Structure* ui;
    QSqlDatabase dbMysqlLocal;
    QStandardItemModel* modelLog;
    QString str_host = "";
    QString str_schema = "";
    QString str_table = "";
    QModelIndex editIndex;
    QVariant previousValue;
    int logCount;

private slots:
    void on_tableData_changed(const QModelIndex& topLeft, const QModelIndex& bottomRight);
    void show_context_menu(const QPoint& pos);
    void add_new(const QModelIndex& index);
    void delete_row();
    void on_buttonUpdateFields_clicked();
};



