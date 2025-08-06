// TunnelSqlManager.cpp
#include "tunnelsqlmanager.h"
#include <QThread>
#include <QTimer>
#include <functions.h>
extern QSqlDatabase dbMysql;

TunnelSqlManager::TunnelSqlManager(QObject *parent)
    : QObject(parent)
{
}

TunnelSqlManager::~TunnelSqlManager()
{
    for (const QString &id : sshTunnels.keys()) {
        desconectar(id);
    }
}

bool TunnelSqlManager::conectar(const QString &id, int &porta,
                                     QString usuarioSsh,
                                     QString servidorSsh,
                                     QString portaSsh,
                                     QString senhaSsh,
                                     QString keyfileSsh,
                                     QString servidorMysql,
                                     QString portaMysql,
                                     QString usuarioMysql,
                                     QString senhaMysql,
                                     QString banco)
{
    if (sqlConnections.contains(id) && sqlConnections[id].isOpen()) {
        qDebug() << "Conexão já existente para" << id;
        return true;
    }

    if (portaMysql == "")
    {
        portaMysql = "22";
    }
    if (portaSsh == "")
    {
        portaSsh = "22";
    }

    // Criar túnel SSH
    int portaLocal = porta + sshTunnels.size();
    porta++;
    if (!sshTunnels.contains(id)) {

        // tunnel: ssh -o ConnectTimeout=60 -o ServerAliveInterval=30 -o ServerAliveCountMax=5 -T -L 3307:localhost:3306 usuario@servidor_ssh

        QProcess *tunnel = new QProcess(this);

        QString key = "";
        if (keyfileSsh!="")
        {
            key = QString("-i%1").arg(keyfileSsh);
        }

        QString portaArg = QString("%1:%2:%3").arg(QString::number(portaLocal), servidorMysql, portaMysql);
        QString destino = QString("%1@%2").arg(usuarioSsh, servidorSsh);
        QString porta = QString("-p%1").arg(portaSsh);

        if (senhaSsh == "")
        {
            QStringList args { "-o", "ConnectTimeout=20", "-o", "ServerAliveInterval=30", "-o", "ServerAliveCountMax=5", "-T", "-L", portaArg, key, porta , destino};
            // qDebug() << "ssh" << args;
            tunnel->start("ssh", args);
        } else {
            // sshpass -p 'senhaAqui' ssh -T -L 3307:localhost:3306 usuario@servidor_ssh
            QString comSenha = QString("'%1'").arg(senhaSsh);
            QStringList args { "-p", comSenha,"ssh", "-o", "ConnectTimeout=20", "-o", "ServerAliveInterval=30", "-o", "ServerAliveCountMax=5", "-T", "-L", portaArg, key, porta , destino};
            // qDebug() << "sshpass" << args;
            tunnel->start("sshpass", args);
        }

        if (!tunnel->waitForStarted(20000)) {
            qWarning() << "Erro ao iniciar túnel SSH para" << id << ":" << tunnel->errorString();
            delete tunnel;
            return false;
        }

        connect(tunnel, &QProcess::readyReadStandardError, this, [=]() {
            qDebug() << tunnel->readAllStandardError();
        });

        sshTunnels[id] = tunnel;
        qDebug() << "Túnel SSH iniciado para" << id;
    }
    QThread::sleep(2);
    // if (!QSqlDatabase::contains("mysql_connection_"+id)) {
    //     QSqlDatabase dbMysql = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection_"+id);
        dbMysql.setHostName("127.0.0.1");
        dbMysql.setPort(portaLocal);
        // dbMysql.setDatabaseName(banco);
        dbMysql.setDatabaseName("gadmin");
        dbMysql.setUserName(usuarioMysql);
        dbMysql.setPassword(senhaMysql);
        qDebug() <<  "Conectando MySQL..." << QString::number(portaLocal) << banco << usuarioMysql << senhaMysql;
        if (!dbMysql.open()) {
            qWarning() << "Erro ao conectar ao MySQL via túnel para" << id << ":" << dbMysql.lastError().text();
            desconectar(id);
            return false;
        }
        sqlConnections[id] = dbMysql;
        qDebug() << "Conexão MySQL aberta para" << id;
    // }

    return true;
}

void TunnelSqlManager::desconectar(const QString &id)
{
    if (sqlConnections.contains(id)) {
        sqlConnections[id].close();
        QSqlDatabase::removeDatabase(id);
        sqlConnections.remove(id);
        qDebug() << "Conexão SQL encerrada para" << id;
    }

    if (sshTunnels.contains(id)) {
        sshTunnels[id]->kill();
        sshTunnels[id]->waitForFinished();
        delete sshTunnels[id];
        sshTunnels.remove(id);
        qDebug() << "Túnel SSH encerrado para" << id;
    }
}

QSqlDatabase TunnelSqlManager::obterConexao(const QString &id) const
{
    if (sqlConnections.contains(id)) {
        return sqlConnections.value(id);
    }
    return QSqlDatabase();
}
