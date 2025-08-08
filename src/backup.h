#pragma once

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDateTime>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QSqlDatabase>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QTableView>
#include <QVBoxLayout>
#include <QVector>

#include "two_checkbox_delegate.h"

class QLineEdit;
class QListView;
class QPushButton;
class TwoCheckboxListModel;

struct ItemData {
    QString label;
    bool checkA;
    bool checkB;
};

class Backup : public QDialog
{
    Q_OBJECT
public:
    explicit Backup(const QString& host, const QString& schema, QWidget* parent = nullptr);
    void refresh_conns();
    void refresh_tables();

private slots:
    void chooseFile();
    void onFavorite();
    void onConfirm();
    void onCancel();
    void onHeaderClicked(int section);
    void onSchemaEditTextChanged(const QString& newText);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QLineEdit* lineEdit;
    QTableView* tableView;
    QPushButton* btnBrowse;
    QPushButton* btnFavorite;
    QPushButton* btnCancel;
    QPushButton* btnConfirm;
    QComboBox* connList;
    QLineEdit* schemaEdit;
    TwoCheckboxDelegate* checkboxDelegate;
    QStandardItemModel* model;
    QString dumpFile = "";
    QString bkp_host, bkp_schema;
    QProgressBar* progressBar;
    QProgressBar* progressBar2;
    bool abort = false;
    bool running = false;
};
