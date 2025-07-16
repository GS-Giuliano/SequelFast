#include "users.h"
#include "ui_users.h"

#include <QDebug>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QAbstractItemModel>
#include <QMessageBox>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlTableModel>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QStyledItemDelegate>

#include <functions.h>

extern QJsonArray connections;
extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;
extern QString currentTheme;


class CheckBoxDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override {
        return nullptr;  // evita entrada de texto
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        bool checked = index.data().toString() == "Y";
        QStyleOptionButton checkbox;
        checkbox.state |= QStyle::State_Enabled;
        checkbox.state |= checked ? QStyle::State_On : QStyle::State_Off;
        checkbox.rect = option.rect.adjusted(4, 4, -4, -4);
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkbox, painter);
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonDblClick) {

            QString current = index.data().toString();
            QString updated = (current == "Y") ? "N" : "Y";
            model->setData(index, updated, Qt::EditRole);
            return true;
        }
        return false;
    }
};


Users::Users(QString &host, QString &schema, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Users)
{
    usr_host = host;
    usr_schema = schema;

    this->setWindowTitle(usr_host +" • "+usr_schema );

    ui->setupUi(this);

    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView, &QTableView::customContextMenuRequested,
            this, &Users::show_context_menu);


    refresh_users();
}

Users::~Users()
{
    delete ui;
}

void Users::refresh_users()
{
    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + usr_host);

    // Usar QSqlTableModel diretamente com a tabela mysql.user
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("mysql.user");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();

    // Colunas que queremos exibir
    QList<QString> columns = {"User", "Host", "Select_priv", "Insert_priv", "Update_priv",
                              "Delete_priv", "Create_priv", "Drop_priv", "Reload_priv",
                              "Shutdown_priv", "Process_priv", "File_priv", "Grant_priv",
                              "References_priv", "Index_priv", "Alter_priv", "Show_db_priv",
                              "Super_priv", "Create_tmp_table_priv", "Lock_tables_priv",
                              "Execute_priv", "Repl_slave_priv", "Repl_client_priv",
                              "Create_view_priv", "Show_view_priv", "Create_routine_priv",
                              "Alter_routine_priv", "Create_user_priv", "Event_priv",
                              "Trigger_priv", "Create_tablespace_priv"};

    // Oculta todas as colunas, depois mostra só as necessárias
    for (int i = 0; i < model->columnCount(); ++i)
        ui->tableView->setColumnHidden(i, true);

    for (const QString &col : columns) {
        int idx = model->fieldIndex(col);
        if (idx >= 0)
            ui->tableView->setColumnHidden(idx, false);
    }

    // Proxy para filtro
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->tableView->setModel(proxy);
    ui->tableView->resizeColumnsToContents();

    // Aplica delegate às colunas de permissões
    QStringList checkCols = {"Select_priv", "Insert_priv", "Update_priv", "Delete_priv",
                             "Create_priv", "Drop_priv", "Reload_priv",
                             "Shutdown_priv", "Process_priv", "File_priv", "Grant_priv",
                             "References_priv", "Index_priv", "Alter_priv", "Show_db_priv",
                             "Super_priv", "Create_tmp_table_priv", "Lock_tables_priv",
                             "Execute_priv", "Repl_slave_priv", "Repl_client_priv",
                             "Create_view_priv", "Show_view_priv", "Create_routine_priv",
                             "Alter_routine_priv", "Create_user_priv", "Event_priv",
                             "Trigger_priv", "Create_tablespace_priv", "account_locked",
                             "Create_role_priv", "Drop_role_priv"};
    for (const QString &col : checkCols) {
        int sourceColumn = model->fieldIndex(col);
        int proxyColumn = proxy->mapFromSource(model->index(0, sourceColumn)).column();
        ui->tableView->setItemDelegateForColumn(proxyColumn, new CheckBoxDelegate(this));
    }

    // Conecta o sinal para executar FLUSH PRIVILEGES após edição
    connect(model, &QSqlTableModel::dataChanged, this,
            [db, model, checkCols](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &) {
                for (int col = topLeft.column(); col <= bottomRight.column(); ++col) {
                    QString fieldName = model->headerData(col, Qt::Horizontal).toString();
                    if (checkCols.contains(fieldName)) {
                        QSqlQuery flush(db);
                        if (!flush.exec("FLUSH PRIVILEGES")) {
                            qWarning() << "Erro ao executar FLUSH PRIVILEGES:" << flush.lastError().text();
                        } else {
                            // qDebug() << "FLUSH PRIVILEGES executado após alteração em" << fieldName;
                        }
                        break; // executa apenas uma vez por alteração
                    }
                }
            }
    );
}

void Users::create_user_dialog(QWidget *parent, const QString &connectionName)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Create New User");

    QFormLayout *layout = new QFormLayout(&dialog);

    QLineEdit *editName = new QLineEdit();
    QLineEdit *editHost = new QLineEdit();
    QLineEdit *editPassword = new QLineEdit();
    editPassword->setEchoMode(QLineEdit::Password);

    editHost->setText("localhost");

    layout->addRow("Username:", editName);
    layout->addRow("Host:", editHost);
    layout->addRow("Password:", editPassword);

    // Radio buttons for role selection
    QRadioButton *radioReadOnly = new QRadioButton("Read only");
    QRadioButton *radioSoftware = new QRadioButton("Software");
    QRadioButton *radioAdmin = new QRadioButton("Admin");
    radioReadOnly->setChecked(true);  // default

    QVBoxLayout *roleLayout = new QVBoxLayout();
    roleLayout->addWidget(radioReadOnly);
    roleLayout->addWidget(radioSoftware);
    roleLayout->addWidget(radioAdmin);

    layout->addRow("Access Level:", roleLayout);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    QString user = editName->text().trimmed();
    QString host = editHost->text().trimmed();
    QString password = editPassword->text();

    if (user.isEmpty() || host.isEmpty()) {
        QMessageBox::warning(parent, "Input Error", "Username and Host cannot be empty.");
        return;
    }

    QStringList permissions;

    if (radioReadOnly->isChecked()) {
        permissions << "SELECT" << "SHOW DATABASES";
    } else if (radioSoftware->isChecked()) {
        permissions << "SELECT" << "INSERT" << "UPDATE" << "DELETE" << "SHOW DATABASES";
    } else if (radioAdmin->isChecked()) {
        permissions << "ALL PRIVILEGES";
    }

    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + usr_host);
    QSqlQuery query(db);

    // Cria o usuário
    QString createUserSQL = QString("CREATE USER '%1'@'%2' IDENTIFIED BY '%3';")
                                .arg(user, host, password);
    if (!query.exec(createUserSQL)) {
        QMessageBox::critical(parent, "Error", "Failed to create user:\n" + query.lastError().text());
        return;
    }

    // Aplica as permissões
    QString perms = permissions.join(", ");
    QString grantSQL = QString("GRANT %1 ON *.* TO '%2'@'%3';")
                           .arg(perms, user, host);
    if (!query.exec(grantSQL)) {
        QMessageBox::critical(parent, "Error", "Failed to grant privileges:\n" + query.lastError().text());
        return;
    }

    // Aplica as permissões na prática
    if (!query.exec("FLUSH PRIVILEGES;")) {
        QMessageBox::warning(parent, "Warning", "User created, but FLUSH PRIVILEGES failed:\n" + query.lastError().text());
    } else {
        refresh_users();
        // QMessageBox::information(parent, "Success", "User created successfully.");
    }
}

void Users::delete_selected_user()
{
    QItemSelectionModel *selection = ui->tableView->selectionModel();

    if (!selection || !selection->hasSelection()) {
        QMessageBox::warning(this, "Delete User", "Please select a user to delete.");
        return;
    }

    QModelIndex indexUser = selection->currentIndex();
    QModelIndex indexHost;

    // Tenta localizar as colunas "User" e "Host" corretamente via modelo fonte
    QAbstractItemModel *proxyModel = ui->tableView->model();
    QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel *>(proxyModel);
    if (!proxy) {
        QMessageBox::critical(this, "Error", "Invalid model type.");
        return;
    }

    QSqlTableModel *sourceModel = qobject_cast<QSqlTableModel *>(proxy->sourceModel());
    if (!sourceModel) {
        QMessageBox::critical(this, "Error", "Underlying model is not QSqlTableModel.");
        return;
    }

    int colUser = sourceModel->fieldIndex("User");
    int colHost = sourceModel->fieldIndex("Host");

    QModelIndex sourceIndex = proxy->mapToSource(indexUser);
    QString user = sourceModel->data(sourceModel->index(sourceIndex.row(), colUser)).toString();
    QString host = sourceModel->data(sourceModel->index(sourceIndex.row(), colHost)).toString();

    if (user.isEmpty() || host.isEmpty()) {
        QMessageBox::warning(this, "Delete User", "Could not retrieve User or Host.");
        return;
    }

    QMessageBox msgBox;
    msgBox.setStyleSheet(
        "QLabel:last {"
        "   padding-right: 20px;"
        "   min-height: 30px;"
        "   min-width: 280px;"
        "}"
        "QPushButton {"
        "    padding: 4px;"
        "    margin: 20px;"
        "    min-height: 18px;"
        "    min-width: 80px;"
        "}"
        );
    msgBox.setWindowTitle("Confirm Delete");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    QString message = QString("Are you sure you want to delete user\n\"%1\"?").arg(user);
    msgBox.setText(message);

    QMessageBox::StandardButton reply = static_cast<QMessageBox::StandardButton>(msgBox.exec());

    // QMessageBox::StandardButton reply = QMessageBox::question(this, "Confirm Deletion",
    //                                                           QString("Are you sure you want to delete the user:\n\n%1@%2 ?").arg(user, host),
    //                                                           QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
        return;

    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + usr_host);
    QSqlQuery query(db);

    QString dropSQL = QString("DROP USER '%1'@'%2';").arg(user, host);
    if (!query.exec(dropSQL)) {
        QMessageBox::critical(this, "Error", "Failed to delete user:\n" + query.lastError().text());
        return;
    }

    if (!query.exec("FLUSH PRIVILEGES;")) {
        QMessageBox::warning(this, "Warning", "User deleted, but FLUSH PRIVILEGES failed:\n" + query.lastError().text());
    }

    // QMessageBox::information(this, "Success", "User deleted successfully.");

    refresh_users();  // Atualiza a lista após exclusão
}

void Users::show_context_menu(const QPoint &pos)
{
    QMenu menu(this);
    QAction *tableAdd = menu.addAction("Add");
    QAction *tableDelete = menu.addAction("Delete");

    QAction *selectedAction = menu.exec(ui->tableView->viewport()->mapToGlobal(pos));

    if (selectedAction == tableAdd) {
        create_user_dialog(this, usr_host);
    }
    else if (selectedAction == tableDelete) {
        delete_selected_user();
    }

}



void Users::on_actionRefresh_triggered()
{
    refresh_users();
}

void Users::on_actionNew_triggered()
{
    create_user_dialog(this, usr_host);
}


void Users::on_actionDelete_triggered()
{
    delete_selected_user();
}

