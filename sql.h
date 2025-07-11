#ifndef SQL_H
#define SQL_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QTableView>

namespace Ui {
class Sql;
}

class Sql : public QMainWindow
{
    Q_OBJECT

public:
    explicit Sql(const QString &host, const QString &schema, const QString &table, const QString &color, QWidget *parent = nullptr);
    ~Sql();
    void query2TableView(QTableView *tableView, const QString &queryStr, const QString &comando);
    void setInterfaceSize(int increase);
    void refresh_structure();
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

    bool on_tableData_edit_trigger(QString &id, QString &fieldName, QString &newValue);

    void show_context_menu(const QPoint &pos);

    void on_tableClone_triggered();
    void on_tableDelete_triggered();
    void on_tableCopyInsert_triggered();
    void on_tableCopyUpdate_triggered();
    void on_tableCopyCsv_triggered();
    void on_tableCopyGfwForm_triggered();
    void on_tableCopyGfwTable_triggered();
    void on_tableCopyLaravelForm_triggered();
    void on_tableCopyLaravelTable_triggered();



private:
    Ui::Sql *ui;
    QSqlDatabase dbMysqlLocal;
    QString sql_host;
    QString sql_schema;
    QString sql_table;
    QString sql_color;
    QLineEdit *edit;
    QLineEdit *editTimes;
    QPushButton *button;
    QTimer *timer;
    QSqlRecord currentRecord;

    bool editEnabled = false;
    bool hasId = false;
    bool hasJoin = false;
    bool hasSubquery = false;
    int idPosition = -1;

    QString tableName = "", tableAlias = "";
    QStringList selectFields;
    QStringList whereFields;
    QStringList orderByFields;

};

#endif // SQL_H
