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

/**
 * @brief SQL Editor and Query Execution Window
 * 
 * The Sql class provides a comprehensive SQL editor with syntax highlighting,
 * auto-completion, query execution, and data editing capabilities for MySQL/MariaDB databases.
 * It supports macro processing, favorites management, and various data export formats.
 */
class Sql : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor for SQL Editor window
     * @param host Database host connection string
     * @param schema Database schema/database name
     * @param table Default table name (can be empty)
     * @param color Connection color identifier for UI theming
     * @param favName Favorite query name (if opening from favorites)
     * @param favValue Favorite query content (if opening from favorites)
     * @param run Whether to immediately execute the favorite query
     * @param parent Parent widget
     */
    explicit Sql(const QString& host, const QString& schema, const QString& table,
        const QString& color, const QString& favName, const QString& favValue,
        const bool& run, QWidget* parent = nullptr);
    
    /**
     * @brief Destructor - cleans up resources and database connections
     */
    ~Sql();

    /**
     * @brief Execute SQL query and populate table view with results
     * @param tableView Target table view widget to display results
     * @param queryStr SQL query string to execute
     * @param comando Command type identifier for logging/tracking
     */
    void query2TableView(QTableView* tableView, const QString& queryStr, const QString& comando);
    
    /**
     * @brief Adjust interface font sizes
     * @param increase Font size increment (can be negative to decrease)
     */
    void setInterfaceSize(int increase);
    
    /**
     * @brief Refresh database structure information for auto-completion
     */
    void refresh_structure();
    
    /**
     * @brief Format SQL text with proper indentation and spacing
     */
    void formatSqlText();
    
    /**
     * @brief Extract macro field definitions from SQL query string
     * @param queryStr SQL query containing macro field definitions
     * @return Vector of MacroField objects representing found macro fields
     */
    QVector<MacroField> extractFields(const QString& queryStr);
    
    /**
     * @brief Process SQL query by resolving macro fields with user input
     * @param queryStr SQL query string containing macro field placeholders
     * @param parent Parent widget for input dialogs
     * @return Processed SQL query with macro fields resolved to actual values
     */
    QString processQueryWithMacros(QString queryStr, QWidget* parent);
    
    /**
     * @brief Initialize and configure SQL auto-completion system
     */
    void setupSqlCompleter();

private slots:
    /**
     * @brief Display status message in the UI
     * @param msg Message text to display
     */
    void statusMessage(QString msg);
    
    /**
     * @brief Handle SQL query execution action trigger
     */
    void on_actionRun_triggered();
    
    /**
     * @brief Handle SQL text formatting action trigger
     */
    void on_actionFormat_triggered();
    
    /**
     * @brief Handle font size increase action trigger
     */
    void on_actionIncrease_triggered();
    
    /**
     * @brief Handle font size decrease action trigger
     */
    void on_actionReduce_triggered();
    
    /**
     * @brief Handle save query to favorites action trigger
     */
    void on_actionSave_triggered();
    
    /**
     * @brief Handle query execution timer tick for performance monitoring
     */
    void on_timer_tick();
    
    /**
     * @brief Handle custom button click events
     */
    void on_button_clicked();
    
    /**
     * @brief Handle table data cell edit operations
     * @param id Primary key value of the row being edited
     * @param fieldName Name of the field being edited
     * @param newValue New value for the field
     * @return True if edit was successful, false otherwise
     */
    bool on_tableData_edit_trigger(QString& id, QString& fieldName, QString& newValue);
    
    /**
     * @brief Handle append new row to table action
     */
    void on_tableAppend_triggered();
    
    /**
     * @brief Handle clone/duplicate selected row action
     */
    void on_tableClone_triggered();
    
    /**
     * @brief Handle delete selected rows action
     */
    void on_tableDelete_triggered();
    
    /**
     * @brief Handle copy selected rows as INSERT statements
     */
    void on_tableCopyInsert_triggered();
    
    /**
     * @brief Handle copy selected rows as UPDATE statements
     */
    void on_tableCopyUpdate_triggered();
    
    /**
     * @brief Handle copy selected rows as CSV format
     */
    void on_tableCopyCsv_triggered();
    
    /**
     * @brief Handle generation of CRUD operations for GFW framework
     */
    void on_tableCRUDGfw_triggered();
    
    /**
     * @brief Handle generation of CRUD operations for Laravel framework
     */
    void on_tableCRUDLaravel_triggered();
    
    /**
     * @brief Handle favorites management dialog action
     */
    void on_actionFavorites_triggered();
    
    /**
     * @brief Handle macros management dialog action
     */
    void on_actionMacros_triggered();
    
    /**
     * @brief Display context menu at specified position
     * @param pos Screen position where context menu should appear
     */
    void show_context_menu(const QPoint& pos);

private:
    /**
     * @brief Get model for table names auto-completion
     * @return Pointer to model containing available table names
     */
    QAbstractItemModel* getTableModel() const;
    
    /**
     * @brief Get model for column names auto-completion for specific table
     * @param table Table name to get columns for
     * @return Pointer to model containing column names for the specified table
     */
    QAbstractItemModel* getColumnModel(const QString& table) const;

    // UI Components
    Ui::Sql* ui;                                    ///< User interface components
    QCompleter* sqlCompleter = nullptr;             ///< SQL auto-completion engine
    QStringListModel* sqlCompleterModel = nullptr; ///< Model for completion suggestions

    // Database Connection
    QSqlDatabase dbMysqlLocal;                      ///< Local database connection instance
    QString sql_host;                               ///< Database host connection string
    QString sql_schema;                             ///< Current database schema name
    QString sql_table;                              ///< Default table name
    QString sql_color;                              ///< Connection color identifier for UI theming

    // UI Controls
    QLineEdit* edit;                                ///< Query input line edit widget
    QLineEdit* editTimes;                           ///< Query execution times input widget
    QPushButton* button;                            ///< Custom action button
    QTimer* timer;                                  ///< Query execution timer for performance monitoring
    QSqlRecord currentRecord;                       ///< Current database record structure

    // Query Analysis Flags
    bool editEnabled = false;                       ///< Whether table data editing is enabled
    bool hasId = false;                            ///< Whether query result has primary key column
    bool hasJoin = false;                          ///< Whether query contains JOIN operations
    bool hasSubquery = false;                      ///< Whether query contains subqueries
    int idPosition = -1;                           ///< Position of primary key column in result set
    int rowsAffected = 0;                          ///< Number of rows affected by last query

    // Query Information
    QString databaseName = "";                      ///< Current database name
    QString tableName = "", tableAlias = "";       ///< Current table name and alias
    QString queryTimer = "";                        ///< Query execution time string
    QString favoriteName = "";                      ///< Name of current favorite query
    QStringList selectFields;                       ///< List of selected fields in current query
    QStringList whereFields;                        ///< List of fields used in WHERE clause
    QStringList orderByFields;                      ///< List of fields used in ORDER BY clause

protected:
    /**
     * @brief Handle keyboard events for custom shortcuts and navigation
     * @param event Keyboard event to process
     */
    void keyPressEvent(QKeyEvent* event) override;
};

