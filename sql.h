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
    explicit Sql(const QString &host, const QString &schema, const QString &table, QWidget *parent = nullptr);
    ~Sql();
    void formatSqlText();

private slots:
    void on_actionRun_triggered();

    void on_actionFormat_triggered();

private:
    Ui::Sql *ui;
};

#endif // SQL_H
