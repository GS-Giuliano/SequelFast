#pragma once

#include <QAbstractItemModel>
#include <QAction>
#include <QClipboard>
#include <QCompleter>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QString>
#include <QStringList>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

#include <functions.h>
#include "texteditcompleter.h"
#include "macroinputdialog.h"
#include "macroformatdialog.h"

namespace Ui {
    class Sql;
}

class Sql : public QMainWindow
{
    Q_OBJECT

public:
    explicit Sql(const QString& host, const QString& schema, const QString& table,
        const QString& color, const QString& favName, const QString& favValue,
        const bool& run, QWidget* parent = nullptr);
    ~Sql();

    void query2TableView(QTableView* tableView, const QString& queryStr, const QString& comando);
    void setInterfaceSize(int increase);
    void refresh_structure();
    void formatSqlText();
    QVector<MacroField> extractFields(const QString& queryStr);
    QString processQueryWithMacros(QString queryStr, QWidget* parent);
    void setupSqlCompleter();

private slots:
    void statusMessage(QString msg);
    void on_actionRun_triggered();
    void on_actionFormat_triggered();
    void on_actionIncrease_triggered();
    void on_actionReduce_triggered();
    void on_actionSave_triggered();
    void on_timer_tick();
    void on_button_clicked();
    bool on_tableData_edit_trigger(QString& id, QString& fieldName, QString& newValue);
    void on_tableAppend_triggered();
    void on_tableClone_triggered();
    void on_tableDelete_triggered();
    void on_tableCopyInsert_triggered();
    void on_tableCopyUpdate_triggered();
    void on_tableCopyCsv_triggered();
    void on_tableCRUDGfw_triggered();
    void on_tableCRUDLaravel_triggered();
    void on_actionFavorites_triggered();
    void on_actionMacros_triggered();
    void show_context_menu(const QPoint& pos);

private:
    // Novos métodos para suporte ao autocompletar
    QAbstractItemModel* getTableModel() const;
    QAbstractItemModel* getColumnModel(const QString& table) const;

    Ui::Sql* ui;
    QCompleter* sqlCompleter = nullptr;
    QStringListModel* sqlCompleterModel = nullptr;

    QSqlDatabase dbMysqlLocal;
    QString sql_host;
    QString sql_schema;
    QString sql_table;
    QString sql_color;
    QLineEdit* edit;
    QLineEdit* editTimes;
    QPushButton* button;
    QTimer* timer;
    QSqlRecord currentRecord;

    bool editEnabled = false;
    bool hasId = false;
    bool hasJoin = false;
    bool hasSubquery = false;
    int idPosition = -1;
    int rowsAffected = 0;

    QString databaseName = "";
    QString tableName = "", tableAlias = "";
    QString queryTimer = "";
    QString favoriteName = "";
    QStringList selectFields;
    QStringList whereFields;
    QStringList orderByFields;

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

