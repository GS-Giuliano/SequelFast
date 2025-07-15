#include <QMainWindow>
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
#include <QMap>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

QJsonArray connections;

QString currentTheme = "dark";

QJsonArray colors;

QJsonArray colorThemes = QJsonArray{
    QJsonObject{
        {"theme", "light"},
        {"colors", QJsonArray{
                       QJsonObject{{"name", "white"},  {"rgb", "#FFFFFF"}},
                       QJsonObject{{"name", "brown"},  {"rgb", "#EDBB99"}},
                       QJsonObject{{"name", "red"},    {"rgb", "#FFBAAB"}},
                       QJsonObject{{"name", "purple"}, {"rgb", "#EBCAFF"}},
                       QJsonObject{{"name", "blue"},   {"rgb", "#CBE8FF"}},
                       QJsonObject{{"name", "green"},  {"rgb", "#DBFBD6"}},
                       QJsonObject{{"name", "yellow"}, {"rgb", "#FCF3CF"}},
                       QJsonObject{{"name", "orange"}, {"rgb", "#FFE6BA"}},
                       QJsonObject{{"name", "grey"},   {"rgb", "#D7DBDD"}}
                   }}
    },
    QJsonObject{
        {"theme", "dark"},
        {"colors", QJsonArray{
                       QJsonObject{{"name", "white"},  {"rgb", "#09131D"}},
                       QJsonObject{{"name", "brown"},  {"rgb", "#63503E"}},
                       QJsonObject{{"name", "red"},    {"rgb", "#612C2C"}},
                       QJsonObject{{"name", "purple"}, {"rgb", "#55416B"}},
                       QJsonObject{{"name", "blue"},   {"rgb", "#2F4A57"}},
                       QJsonObject{{"name", "green"},  {"rgb", "#355535"}},
                       QJsonObject{{"name", "yellow"}, {"rgb", "#6F642B"}},
                       QJsonObject{{"name", "orange"}, {"rgb", "#754823"}},
                       QJsonObject{{"name", "grey"},   {"rgb", "#424242"}}
                   }}
    }
};


QSqlDatabase dbPreferences;
QSqlDatabase dbMysql;
QString dbPath = "";
QString dbName = "preferences.db";

QString actual_host = "";
QString actual_schema = "";
QString actual_table = "";
QString actual_color = "";

int pref_sql_limit = 100;
int pref_table_row_height = 40;
int pref_table_font_size = 10;
int pref_sql_font_size = 10;

bool prefLoaded = false;


bool openPreferences()
{
    // qDebug() << QSqlDatabase::drivers();
    // qDebug() << "Plugin paths:" << QCoreApplication::libraryPaths();

    if (!dbPreferences.isValid())
    {
        // Caminho recomendado
        QString basePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(basePath);
        QString dbPath = basePath + "/" + dbName;

        // Conectar
        dbPreferences = QSqlDatabase::addDatabase("QSQLITE", "pref_connection");
        // qDebug() << "Preferences path: " << dbPath;
        dbPreferences.setDatabaseName(dbPath);
    }

    if (!dbPreferences.open()) {
        prefLoaded = false;
        qCritical() << "Erro ao abrir o banco de dados SQLite:" << dbPreferences.lastError().text();
        return(false);
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
        QString createTableSql = "CREATE TABLE IF NOT EXISTS prefs ("
                                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                                 "name TEXT NULL,"
                                 "value TEXT NULL,"
                                 "type TEXT NULL"
                                 ")";
        if (!query.exec(createTableSql)) {
            qCritical() << "Erro ao criar a tabela 'pref':" << query.lastError().text();
            dbPreferences.close();
        } else {
            if (!query.exec("SELECT COUNT(id) ttl FROM prefs")) {
                qCritical() << "Erro ao consultar dados:" << query.lastError().text();
            } else {
                while (query.next()) {
                    int id = query.value("ttl").toInt();
                    if (id == 0)
                    {
                        if (!query.exec(QString("INSERT INTO prefs (name, type, value) VALUES ('sql_limit', 'int', '100')"))) {
                            qWarning() << "Erro ao inserir host:" << query.lastError().text();
                        }
                        if (!query.exec(QString("INSERT INTO prefs (name, type, value) VALUES ('table_row_height', 'int', '40')"))) {
                            qWarning() << "Erro ao inserir host:" << query.lastError().text();
                        }
                        if (!query.exec(QString("INSERT INTO prefs (name, type, value) VALUES ('table_font_size', 'int', '10')"))) {
                            qWarning() << "Erro ao inserir host:" << query.lastError().text();
                        }
                        if (!query.exec(QString("INSERT INTO prefs (name, type, value) VALUES ('sql_font_size', 'int', '12')"))) {
                            qWarning() << "Erro ao inserir host:" << query.lastError().text();
                        }
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
    prefLoaded = true;
    return(true);
    // dbPreferences.close();

}

void updateIntPreference(QString name, int value)
{
    QSqlQuery query(QSqlDatabase::database("pref_connection"));
    if (!query.exec("SELECT name FROM prefs WHERE name = '" + name + "'")) {
        qCritical() << "Erro ao consultar dados:" << query.lastError().text();
    } else {
        if (query.last())
        {
            QString updateSql = "UPDATE prefs SET type  = :type, value = :value WHERE name =:name";
            query.prepare(updateSql); // Prepara a consulta para inserção segura (evita SQL injection)

            query.bindValue(":name", name);
            query.bindValue(":type", "int");
            query.bindValue(":value", value);

            if (!query.exec()) {
                qWarning() << "Erro ao inserir host:" << query.lastError().text();
            }

        } else {
            QString insertSql = "INSERT INTO prefs (name, type, value) VALUES (:name, :type, :value)";
            query.prepare(insertSql); // Prepara a consulta para inserção segura (evita SQL injection)

            query.bindValue(":name", name);
            query.bindValue(":type", "int");
            query.bindValue(":value", value);

            if (!query.exec()) {
                qWarning() << "Erro ao inserir host:" << query.lastError().text();
            }
        }
    }
}

int getIntPreference(QString name)
{
    int value = 0;
    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    query.prepare("SELECT value FROM prefs WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qCritical() << "Erro ao consultar dados:" << query.lastError().text();
        return value;
    }

    // Verifica se retornou algum resultado
    if (query.next()) {
        value = query.value(0).toInt();
    } else {
        qDebug() << "Preferência não encontrada para:" << name;
    }
    return(value);
}

QString getStringPreference(QString name)
{
    QString value = "";
    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    query.prepare("SELECT value FROM prefs WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qCritical() << "Erro ao consultar dados:" << query.lastError().text();
        return value;
    }

    if (query.next()) {
        value = query.value(0).toString();
    } else {
        qDebug() << "Preferência não encontrada para:" << name;
    }
    return(value);
}

QString setStringPreference(QString name, QString value)
{
    QSqlQuery query(QSqlDatabase::database("pref_connection"));

    query.prepare("SELECT value FROM prefs WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec()) {
        qCritical() << "Erro ao consultar dados:" << query.lastError().text();
        return value;
    }

    if (query.next()) {
        QString updateSql = "UPDATE prefs SET value = :value WHERE name = :name";
        query.prepare(updateSql);

        query.bindValue(":name", name);
        query.bindValue(":value", value);

        if (!query.exec()) {
            qWarning() << "Erro ao inserir host:" << query.lastError().text();
        }
    } else {
        qDebug() << "Preferência não encontrada para:" << name;
        QString insertSql = "INSERT INTO prefs (name, type, value) VALUES (:name, 'string', :value)";
        query.prepare(insertSql);

        query.bindValue(":name", name);
        query.bindValue(":value", value);

        if (!query.exec()) {
            qWarning() << "Erro ao inserir host:" << query.lastError().text();
        }

    }
    return(value);
}


void getPreferences()
{
    pref_sql_limit = getIntPreference("sql_limit");
    pref_table_row_height = getIntPreference("table_row_height");
    pref_table_font_size = getIntPreference("table_font_size");
    pref_sql_font_size = getIntPreference("sql_font_size");

    // qDebug() << "pref_sql_limit " << pref_sql_limit ;
    // qDebug() << "pref_table_row_height " << pref_table_row_height ;
    // qDebug() << "pref_table_font_size " << pref_table_font_size ;
    // qDebug() << "pref_sql_font_size " << pref_sql_font_size ;
}

void updatePreferences()
{
    updateIntPreference("sql_limit", pref_sql_limit);
    updateIntPreference("table_row_height", pref_table_row_height);
    updateIntPreference("table_font_size", pref_table_font_size);
    updateIntPreference("sql_font_size", pref_sql_font_size);
}

bool addConnection(QString name,
                   QString color = "",
                   QString host = "",
                   QString user = "",
                   QString pass = "",
                   QString port = "",
                   QString ssh_host = "",
                   QString ssh_user = "",
                   QString ssh_pass = "",
                   QString ssh_port = "",
                   QString ssh_keyfile = "")
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
            QString insertSql = "INSERT INTO conns (name, color, host, user, pass, port, ssh_host, ssh_user, ssh_pass, ssh_port, ssh_keyfile) "
                                "VALUES (:name, :color, :host, :user, :pass, :port, :ssh_host, :ssh_user, :ssh_pass, :ssh_port, :ssh_keyfile)";
            query.prepare(insertSql);

            query.bindValue(":name", name);
            query.bindValue(":color", color);
            query.bindValue(":host", host);
            query.bindValue(":user", user);
            query.bindValue(":pass", pass);
            query.bindValue(":port", port);

            query.bindValue(":ssh_host", ssh_host);
            query.bindValue(":ssh_user", ssh_user);
            query.bindValue(":ssh_pass", ssh_pass);
            query.bindValue(":ssh_port", ssh_port);
            query.bindValue(":ssh_keyfile", ssh_keyfile);


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

QString getRgbFromColorName(const QString &colorName)
{
    for (const QJsonValue &val : colors) {
        QJsonObject obj = val.toObject();
        if (obj["name"].toString() == colorName.toLower()) {
            return obj["rgb"].toString();
        }
    }
    return "#FFFFFF";
}

QStringList extractFieldsWithPrefix(const QStringList &fields, const QString &tableName, const QString &alias) {
    QStringList result;
    for (const QString &field : fields) {
        QString trimmed = field.trimmed();

        // Campo com prefixo (ex: P.nome ou pessoas.nome)
        if (trimmed.contains('.')) {
            QString prefix = trimmed.section('.', 0, 0);
            QString fieldName = trimmed.section('.', 1);

            if ((!alias.isEmpty() && prefix == alias) || prefix == tableName) {
                result << fieldName;
            }
        } else {
            // Campo sem prefixo: assume-se que pertence à tabela principal
            result << trimmed;
        }
    }
    return result;
}


QString extractCurrentQuery(const QString &text, int cursorPos)
{
    if (text.isEmpty() || cursorPos < 0 || cursorPos > text.length())
        return "";

    // Ajuste: se o cursor está no fim exato e o último caractere é ';', retroceder
    if (cursorPos == text.length() && text.endsWith(';'))
        cursorPos--;

    // Encontrar início (ponto e vírgula anterior)
    int start = 0;
    if (cursorPos > 0) {
        int lastSemi = text.lastIndexOf(';', cursorPos - 1);
        start = (lastSemi == -1) ? 0 : lastSemi + 1;
    }

    // Encontrar fim (próximo ponto e vírgula após o cursor)
    int end = text.indexOf(';', cursorPos);
    if (end == -1)
        end = text.length();

    QString query = text.mid(start, end - start).trimmed();
    return query;
}

