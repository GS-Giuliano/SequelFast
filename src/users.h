#ifndef USERS_H
#define USERS_H

#include <QMainWindow>

namespace Ui {
class Users;
}

class Users : public QMainWindow
{
    Q_OBJECT

public:
    explicit Users(QString &host, QString &schema, QWidget *parent);
    ~Users();

    void refresh_users();
    void create_user_dialog(QWidget *parent, const QString &connectionName);
    void delete_selected_user();

private slots:
    void on_actionRefresh_triggered();

    void on_actionNew_triggered();

    void on_actionDelete_triggered();

private:
    Ui::Users *ui;

    QString usr_host;
    QString usr_schema;

};

#endif // USERS_H
