#include "statisticsbackend.h"
#include "globalvariablesmodel.h"

#include <QSqlDatabase>
#include <QSqlQuery>

StatisticsBackend::StatisticsBackend(const QString& host, const QString& schema, QObject* parent)
    : QObject(parent)
    , m_title(host + " • " + schema)
    , m_schemaName(schema)
{
    QSqlDatabase db = QSqlDatabase::database("mysql_connection_" + host);

    QSqlQuery query(db);
    if (query.exec("SELECT * FROM information_schema.SCHEMATA WHERE SCHEMA_NAME = '" + schema + "'")) {
        if (query.next()) {
            m_charSet = query.value("DEFAULT_CHARACTER_SET_NAME").toString();
            m_collation = query.value("DEFAULT_COLLATION_NAME").toString();
            m_encryption = query.value("DEFAULT_ENCRYPTION").toString();

            if (query.exec("SELECT table_schema AS name, ROUND(SUM(data_length + index_length) / 1024 / 1024, 2) size "
                            "FROM information_schema.TABLES "
                            "WHERE table_schema = '" + schema + "' "
                            "GROUP BY table_schema")) {
                if (query.next()) {
                    m_size = query.value("size").toString() + " Mb";
                }
            }

            if (query.exec("SELECT COUNT(*) AS total_tables FROM information_schema.tables "
                            "WHERE table_schema = '" + schema + "'")) {
                if (query.next()) {
                    m_tableCount = query.value("total_tables").toString();
                }
            }
        }
    }

    m_variablesModel = new GlobalVariablesModel(db, this);
}

void StatisticsBackend::onClose()
{
    emit closed();
}
