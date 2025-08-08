#pragma once

#define APP_VERSION "0.1.0"
#define APP_BUILD_DATE __DATE__
#define APP_BUILD_TIME __TIME__

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QStringListModel>

bool openPreferences();
// Conections (MySQL)
QJsonObject getConnection(QString selectedHost);
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
    QString ssh_keyfile = "");
bool deleteConnection(QString name);
bool connMysql(QWidget* parent, QString selectedHost);
// Preferences
void updateIntPreference(QString name, int value);
int getIntPreference(QString name);
QString getStringPreference(QString name);
QString setStringPreference(QString name, QString value);
void getPreferences();
void updatePreferences();
QString getRgbFromColorName(const QString& colorName);
QStringList extractFieldsWithPrefix(const QStringList& fields, const QString& tableName, const QString& alias);
QString extractCurrentQuery(const QString& text, int cursorPos);
bool connectMySQL(const QString selectedHost, QObject* parent = nullptr, const QString prefix = "mysql_connection_");
QString generateCreateTableStatement(const QString& tableName, const QString& connectionName);
QString generateColumnsCsv(const QString& tableName, const QString& connectionName);

