#pragma once

#include <QColorDialog>
#include <QCoreApplication>
#include <QDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace Ui {
    class Connection;
}

class Connection : public QDialog
{
    Q_OBJECT

public:
    explicit Connection(QString selectedHost, QWidget* parent = nullptr);
    ~Connection();
    void saveConnection();

private slots:
    void on_buttonCancel_clicked();

    void on_buttonSave_clicked();

    void on_buttonRemove_clicked();

    void on_buttonConnect_clicked();

    void on_dial_valueChanged(int value);

private:
    Ui::Connection* ui;
};

