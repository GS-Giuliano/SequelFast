#ifndef BATCH_H
#define BATCH_H

#include <QMainWindow>

namespace Ui {
class Batch;
}

class Batch : public QMainWindow
{
    Q_OBJECT

public:
    explicit Batch(QString &host, QString &schema, QWidget *parent);
    ~Batch();

private:
    Ui::Batch *ui;
    QString usr_host;
    QString usr_scheme;
};

#endif // BATCH_H
