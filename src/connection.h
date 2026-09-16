#pragma once

#include <QDialog>
#include <QString>

class ConnectionBackend;
class QQuickWidget;

class Connection : public QDialog
{
    Q_OBJECT

public:
    explicit Connection(QString selectedHost, QWidget* parent = nullptr);
    ~Connection();

private:
    ConnectionBackend* backend;
    QQuickWidget* quickWidget;
};
