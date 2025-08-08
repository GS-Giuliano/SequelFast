// restore.h
#pragma once

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMessageBox>
#include <QObject>
#include <QProgressBar>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

class Restore : public QObject
{
    Q_OBJECT
public:
    explicit Restore(QObject* parent = nullptr);
    void run(QString fileName, const QString& bkp_prefix, const QString& bkp_host, const QString& bkp_schema, QWidget* parent);
private:
    bool showMessage = false;
};


