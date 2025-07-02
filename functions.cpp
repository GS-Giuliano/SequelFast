#include <QDebug>
#include <QMessageBox>
#include <QStringListModel>

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

QJsonArray connections;
QSqlDatabase dbPreferences;
QSqlDatabase dbMysql;
QString dbPath = "preferences.db";

QString actual_host = "";
QString actual_schema = "";
QString actual_table = "";

void openPreferences()
{

    if (!dbPreferences.isValid())
    {
        dbPreferences = QSqlDatabase::addDatabase("QSQLITE", "pref_connection");
        dbPreferences.setDatabaseName(dbPath); // Define o caminho para o arquivo do banco de dados
    }

    if (!dbPreferences.open()) {
        qCritical() << "Erro ao abrir o banco de dados SQLite:" << dbPreferences.lastError().text();
    }

    QSqlQuery query(QSqlDatabase::database("pref_connection"));
    QString createTableSql = "CREATE TABLE IF NOT EXISTS conns ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "name TEXT NULL,"
                             "color TEXT NULL,"
                             "host TEXT NULL,"
                             "port TEXT NULL,"
                             "schema TEXT NULL,"
                             "user TEXT NULL,"
                             "pass TEXT NULL,"
                             "ssh_host TEXT NULL,"
                             "ssh_port TEXT NULL,"
                             "ssh_user TEXT NULL,"
                             "ssh_pass TEXT NULL,"
                             "ssh_keyfile TEXT NULL"
                             ")";
    if (!query.exec(createTableSql)) {
        qCritical() << "Erro ao criar a tabela 'conns':" << query.lastError().text();
        dbPreferences.close();
    } else {
        if (!query.exec("SELECT COUNT(id) ttl FROM conns")) {
            qCritical() << "Erro ao consultar dados:" << query.lastError().text();
        } else {
            while (query.next()) {
                int id = query.value("ttl").toInt();
                if (id == 0)
                {
                    QString insertSql = "INSERT INTO conns (name, host, user, pass, port) VALUES (:name, :host, :user, :pass, :port)";
                    query.prepare(insertSql); // Prepara a consulta para inserção segura (evita SQL injection)

                    query.bindValue(":name", "Localhost");
                    query.bindValue(":host", "127.0.0.1");
                    query.bindValue(":user", "root");
                    query.bindValue(":pass", "");
                    query.bindValue(":port", "3306");

                    if (!query.exec()) {
                        qWarning() << "Erro ao inserir host:" << query.lastError().text();
                    }
                }
            }
        }
    }

    if (!query.exec("SELECT * FROM conns")) {
        qCritical() << "Erro ao consultar dados:" << query.lastError().text();
    } else {
        connections = QJsonArray();
        while (query.next()) {
            QJsonObject obj;
            obj["name"] = query.value("name").toString();
            obj["color"] = query.value("color").toString();
            obj["host"] = query.value("host").toString();
            obj["port"] = query.value("port").toString();
            obj["user"] = query.value("user").toString();
            obj["pass"] = query.value("pass").toString();
            obj["schema"] = query.value("schema").toString();
            obj["ssh_host"] = query.value("ssh_host").toString();
            obj["ssh_port"] = query.value("ssh_port").toString();
            obj["ssh_user"] = query.value("ssh_user").toString();
            obj["ssh_pass"] = query.value("ssh_pass").toString();
            obj["ssh_keyfile"] = query.value("ssh_keyfile").toString();
            connections.append(obj);
        }
    }

    for (const QJsonValue &valor : connections) {
        if (valor.isObject()) {
            QJsonObject item = valor.toObject();
        }
    }

    dbPreferences.close();
}


bool addConnection(QString name)
{
    bool sai = false;
    if (!dbPreferences.isValid())
    {
        dbPreferences = QSqlDatabase::addDatabase("QSQLITE", "pref_connection");
        dbPreferences.setDatabaseName(dbPath); // Define o caminho para o arquivo do banco de dados
    }

    if (!dbPreferences.open()) {
        qCritical() << "Erro ao abrir o banco de dados SQLite:" << dbPreferences.lastError().text();
    }

    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    query.prepare("SELECT * FROM conns WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qCritical() << "Erro ao consultar dados:" << query.lastError().text();
        return(false);
    } else {
        if (!query.last()) {
            QString insertSql = "INSERT INTO conns (name, host, user, pass, port) VALUES (:name, :host, :user, :pass, :port)";
            query.prepare(insertSql);

            query.bindValue(":name", name);
            query.bindValue(":host", "");
            query.bindValue(":user", "");
            query.bindValue(":pass", "");
            query.bindValue(":port", "3306");

            if (!query.exec()) {
                qWarning() << "Erro ao inserir host:" << query.lastError().text();
            } else {
                sai = true;
            }
            openPreferences();
        }
    }
    return(sai);
}

bool deleteConnection(QString name)
{
    bool sai = false;
    if (!dbPreferences.isValid())
    {
        dbPreferences = QSqlDatabase::addDatabase("QSQLITE", "pref_connection");
        dbPreferences.setDatabaseName(dbPath); // Define o caminho para o arquivo do banco de dados
    }

    if (!dbPreferences.open()) {
        qCritical() << "Erro ao abrir o banco de dados SQLite:" << dbPreferences.lastError().text();
    }

    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    query.prepare("SELECT * FROM conns WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qCritical() << "Erro ao consultar dados:" << query.lastError().text();
        return(false);
    } else {
        if (query.last()) {
            QString insertSql = "DELETE FROM conns WHERE name = :name";
            query.prepare(insertSql);
            query.bindValue(":name", name);
            if (!query.exec()) {
                qWarning() << "Erro ao remover host:" << query.lastError().text();
            } else {
                sai = true;
            }
            openPreferences();
        }
    }
    return(sai);
}

QJsonObject getConnection(QString selectedHost)
{
    QJsonObject item;
    for (const QJsonValue &valor : connections) {
        if (valor.isObject()) {
            item = valor.toObject();
            if (item["name"].toVariant().toString() == selectedHost)
            {
                return(item);
            }
        }
    }
    return(item);
}
