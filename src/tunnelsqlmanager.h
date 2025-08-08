#pragma once

#include <QApplication>
#include <QDebug>
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlError>
#include <QThread>
#include <QTimer>

class TunnelSqlManager : public QObject
{
    Q_OBJECT

public:
    explicit TunnelSqlManager(QObject* parent = nullptr);
    ~TunnelSqlManager();

    bool conectar(const QString& id, int& porta,
        QString usuarioSsh,
        QString servidorSsh,
        QString portaSsh,
        QString senhaSsh,
        QString keyfileSsh,
        QString servidorMysql,
        QString portaMysql,
        QString usuarioMysql,
        QString senhaMysql,
        QString banco);

    void desconectar(const QString& id);
    QSqlDatabase obterConexao(const QString& id) const;

private:
    QMap<QString, QProcess*> sshTunnels;
    QMap<QString, QSqlDatabase> sqlConnections;
};


