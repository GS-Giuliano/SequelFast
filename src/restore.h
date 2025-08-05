// restore.h
#ifndef RESTORE_H
#define RESTORE_H

#include <QObject>
#include <QString>
#include <QWidget>

class Restore : public QObject
{
    Q_OBJECT
public:
    explicit Restore(QObject *parent = nullptr);
    void run(QString fileName, const QString &bkp_host, const QString &bkp_schema, QWidget *parent);
private:
    bool showMessage = false;
};

#endif // RESTORE_H
