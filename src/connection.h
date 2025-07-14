#ifndef CONNECTION_H
#define CONNECTION_H

#include <QDialog>

namespace Ui {
class Connection;
}

class Connection : public QDialog
{
    Q_OBJECT

public:
    explicit Connection(QString selectedHost, QWidget *parent = nullptr);
    ~Connection();
    void saveConnection();

private slots:
    void on_buttonCancel_clicked();

    void on_buttonSave_clicked();

    void on_buttonRemove_clicked();

    void on_buttonConnect_clicked();

    void on_dial_valueChanged(int value);

private:
    Ui::Connection *ui;
};

#endif // CONNECTION_H
