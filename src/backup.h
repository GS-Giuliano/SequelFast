#ifndef BACKUP_H
#define BACKUP_H

#include <QDialog>
#include <QVector>
#include <QTableView>
#include <QStandardItemModel>
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
    explicit Backup(const QString &host, const QString &schema, QWidget *parent = nullptr);
    void refresh_tables();

private slots:
    void chooseFile();
    void onConfirm();
    void onCancel();
    void onHeaderClicked(int section);

private:
    QLineEdit *lineEdit;
    QTableView *tableView;
    QPushButton *btnBrowse;
    QPushButton *btnCancel;
    QPushButton *btnConfirm;
    TwoCheckboxDelegate *checkboxDelegate;
    QStandardItemModel *model;

    QString bkp_host, bkp_schema;
};

#endif // BACKUP_H
