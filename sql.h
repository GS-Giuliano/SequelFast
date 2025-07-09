#ifndef SQL_H
#define SQL_H

#include <QMainWindow>

namespace Ui {
class Sql;
}

class Sql : public QMainWindow
{
    Q_OBJECT

public:
    explicit Sql(const QString &host, const QString &schema, const QString &table, const QString &color, QWidget *parent = nullptr);
    ~Sql();
    void setInterfaceSize(int increase);
    void formatSqlText();
    void statusMessage(QString msg);

private slots:
    void on_actionRun_triggered();

    void on_actionFormat_triggered();

    void on_actionIncrease_triggered();

    void on_actionReduce_triggered();

    void on_actionSave_triggered();

    void on_timer_tick();

    void on_button_clicked();

private:
    Ui::Sql *ui;
};

#endif // SQL_H
