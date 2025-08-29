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
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QUndoStack>

#include <functions.h>
#include "texteditcompleter.h"
#include "macroinputdialog.h"
#include "macroformatdialog.h"

namespace Ui {
class Sql;
}

class UpdateCellCommand;

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
    void showChart();

private slots:
    void statusMessage(QString msg);
    void on_actionRun_triggered();
    void on_actionFormat_triggered();
    void on_actionIncrease_triggered();
    void on_actionReduce_triggered();
    void on_actionSave_triggered();
    void handleTimer_tick();
    void handleButton_clicked();
    bool handleTableData_edit_trigger(QString& id, QString& fieldName, QString& newValue);
    void handleTableAppend_triggered();
    void handleTableClone_triggered();
    void handleTableDelete_triggered();
    void handleTableCopyInsert_triggered();
    void handleTableCopyUpdate_triggered();
    void handleTableCopyCsv_triggered();
    void handletableCRUDGfw_triggered();
    void handletableCRUDLaravel_triggered();
    void on_actionFavorites_triggered();
    void on_actionMacros_triggered();
    void show_context_menu(const QPoint& pos);
    void log(QString str);

    // >>> ADIÇÃO: clique no cabeçalho para ordenar
    void on_tableHeader_sectionClicked(int logicalIndex);
    // <<<

    void on_actionAuto_commit_triggered();

    void on_actionCommit_triggered();

    void on_actionChart_triggered();

private:
    friend class UpdateCellCommand; // permite a esta classe acessar privados da sql

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

    QSortFilterProxyModel* tableProxy = nullptr;
    enum SortState { SortNone = 0, SortAsc = 1, SortDesc = 2 };
    int currentSortColumn = -1;
    SortState currentSortState = SortNone;
    enum { OriginalRowRole = Qt::UserRole + 77 };
    void applySortState(int column); 
    void resetSortToOriginalOrder(); 

    QUndoStack* undoStack = nullptr;
    
protected:
    void keyPressEvent(QKeyEvent* event) override;
    QLineEdit *limitEdit;
    QStringList commitCache;
};
