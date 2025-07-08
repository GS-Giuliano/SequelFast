#ifndef STATISTICS_H
#define STATISTICS_H

#include <QDialog>

namespace Ui {
class Statistics;
}

class Statistics : public QDialog
{
    Q_OBJECT

public:
    explicit Statistics(QString &host, QString &schema, QWidget *parent = nullptr);
    ~Statistics();

private:
    Ui::Statistics *ui;

private slots:
    void on_tableView_cellClicked(const QModelIndex &index);

};

#endif // STATISTICS_H
