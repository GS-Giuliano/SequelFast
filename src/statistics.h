#pragma once

#include <QDialog>
#include <QString>

class QQuickWidget;
class StatisticsBackend;

class Statistics : public QDialog
{
    Q_OBJECT

public:
    explicit Statistics(QString& host, QString& schema, QWidget* parent = nullptr);
    ~Statistics();

private:
    StatisticsBackend* backend;
    QQuickWidget* quickWidget;
};
