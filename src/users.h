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
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QStyledItemDelegate>

namespace Ui {
    class Users;
}

class Users : public QMainWindow
{
    Q_OBJECT

public:
    explicit Users(QString& host, QString& schema, QWidget* parent);
    ~Users();

    void refresh_users();
    void create_user_dialog(QWidget* parent, const QString& connectionName);
    void delete_selected_user();


private slots:
    void show_context_menu(const QPoint& pos);

    void on_actionRefresh_triggered();

    void on_actionNew_triggered();

    void on_actionDelete_triggered();

private:
    Ui::Users* ui;

    QString usr_host;
    QString usr_schema;

};


