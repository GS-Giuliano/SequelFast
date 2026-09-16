#include "connectionbackend.h"
#include "functions.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

extern QSqlDatabase dbPreferences;
extern QSqlDatabase dbMysql;
extern QString actual_host;
extern QJsonArray colors;

ConnectionBackend::ConnectionBackend(QString selectedHost, QObject* parent)
    : QObject(parent)
    , m_originalName(selectedHost)
{
    m_name = selectedHost;

    QJsonObject item = getConnection(m_originalName);

    m_host = item["host"].toString();
    m_port = item["port"].toString();
    m_user = item["user"].toString();
    m_pass = item["pass"].toString();
    m_schema = item["schema"].toString();

    m_sshHost = item["ssh_host"].toString();
    m_sshPort = item["ssh_port"].toString();
    m_sshUser = item["ssh_user"].toString();
    m_sshPass = item["ssh_pass"].toString();
    m_sshKeyFile = item["ssh_keyfile"].toString();

    m_shared = item["shared"].toString() == "1";

    m_colorName = item["color"].toString();

    for (int i = 0; i < colors.size(); ++i) {
        QJsonObject obj = colors[i].toObject();
        m_colorList.append(QVariantMap{
            {"name", obj["name"].toString()},
            {"rgb", obj["rgb"].toString()},
        });
        if (obj["name"].toString() == m_colorName) {
            m_colorIndex = i;
        }
    }
}

void ConnectionBackend::setName(const QString& v) { if (m_name != v) { m_name = v; emit nameChanged(); } }
void ConnectionBackend::setHost(const QString& v) { if (m_host != v) { m_host = v; emit hostChanged(); } }
void ConnectionBackend::setPort(const QString& v) { if (m_port != v) { m_port = v; emit portChanged(); } }
void ConnectionBackend::setUser(const QString& v) { if (m_user != v) { m_user = v; emit userChanged(); } }
void ConnectionBackend::setPass(const QString& v) { if (m_pass != v) { m_pass = v; emit passChanged(); } }
void ConnectionBackend::setSchema(const QString& v) { if (m_schema != v) { m_schema = v; emit schemaChanged(); } }
void ConnectionBackend::setSshHost(const QString& v) { if (m_sshHost != v) { m_sshHost = v; emit sshHostChanged(); } }
void ConnectionBackend::setSshPort(const QString& v) { if (m_sshPort != v) { m_sshPort = v; emit sshPortChanged(); } }
void ConnectionBackend::setSshUser(const QString& v) { if (m_sshUser != v) { m_sshUser = v; emit sshUserChanged(); } }
void ConnectionBackend::setSshPass(const QString& v) { if (m_sshPass != v) { m_sshPass = v; emit sshPassChanged(); } }
void ConnectionBackend::setSshKeyFile(const QString& v) { if (m_sshKeyFile != v) { m_sshKeyFile = v; emit sshKeyFileChanged(); } }
void ConnectionBackend::setShared(bool v) { if (m_shared != v) { m_shared = v; emit sharedChanged(); } }

void ConnectionBackend::setColorIndex(int v)
{
    if (v < 0 || v >= m_colorList.size() || m_colorIndex == v) {
        return;
    }
    m_colorIndex = v;
    m_colorName = m_colorList[v].toMap()["name"].toString();
    emit colorIndexChanged();
}

void ConnectionBackend::saveConnection()
{
    if (!dbPreferences.open()) {
        qCritical() << "Erro ao abrir o banco de dados SQLite:" << dbPreferences.lastError().text();
    }

    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    QString updateSql = "UPDATE conns SET shared = :shared, name = :new_name, schema = :new_schema, color = :new_color, host = :new_host, user = :new_user, pass = :new_pass, port = :new_port, ssh_host = :new_sshhost, ssh_user = :new_sshuser, ssh_pass = :new_sshpass, ssh_port = :new_sshport, ssh_keyfile = :new_sshkey WHERE name = :name_to_update";
    query.prepare(updateSql);
    query.bindValue(":new_color", m_colorName);
    query.bindValue(":shared", m_shared ? 1 : 0);
    query.bindValue(":new_name", m_name);
    query.bindValue(":new_schema", m_schema);
    query.bindValue(":new_host", m_host);
    query.bindValue(":new_port", m_port);
    query.bindValue(":new_user", m_user);
    query.bindValue(":new_pass", m_pass);
    query.bindValue(":new_sshhost", m_sshHost);
    query.bindValue(":new_sshport", m_sshPort);
    query.bindValue(":new_sshuser", m_sshUser);
    query.bindValue(":new_sshpass", m_sshPass);
    query.bindValue(":new_sshkey", m_sshKeyFile);

    query.bindValue(":name_to_update", m_originalName);
    if (!query.exec()) {
        qWarning() << "Erro ao salvar host:" << query.lastError().text();
    }
    actual_host = m_name;
    openPreferences();
}

void ConnectionBackend::onCancel()
{
    emit rejected();
}

void ConnectionBackend::onSave()
{
    saveConnection();
    emit accepted();
}

void ConnectionBackend::onRemove()
{
    deleteConnection(m_originalName);
    emit accepted();
}

void ConnectionBackend::onConnect()
{
    saveConnection();
    QJsonObject item = getConnection(m_name);

    dbMysql = QSqlDatabase::addDatabase("QMYSQL", "mysql_connection_" + m_name);

    dbMysql.setHostName(item["host"].toVariant().toString());
    dbMysql.setDatabaseName(item["schema"].toVariant().toString());
    dbMysql.setPort(item["port"].toVariant().toInt());
    dbMysql.setUserName(item["user"].toVariant().toString());
    dbMysql.setPassword(item["pass"].toVariant().toString());

    dbMysql.setConnectOptions(
        "MYSQL_OPT_CONNECT_TIMEOUT=60;"
        "MYSQL_OPT_READ_TIMEOUT=28800;"
        "MYSQL_OPT_WRITE_TIMEOUT=28800;"
        "CLIENT_INTERACTIVE=1;"
        "MYSQL_OPT_RECONNECT=1;"
        "CLIENT_COMPRESS=1;"
        );

    if (!dbMysql.open()) {
        emit connectionFailed(dbMysql.lastError().text());
        return;
    }

    QSqlQuery q(dbMysql);
    q.exec("SET SESSION net_read_timeout=28800");
    q.exec("SET SESSION net_write_timeout=28800");
    q.exec("SET SESSION innodb_lock_wait_timeout=900");

    emit connectionSucceeded();
}
