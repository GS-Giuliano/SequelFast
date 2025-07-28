#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define APP_VERSION "0.1.0"
#define APP_BUILD_DATE __DATE__
#define APP_BUILD_TIME __TIME__

#include <QDebug>

#include <QMainWindow>
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


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
bool connMysql(QWidget *parent, QString selectedHost);
// Preferences
void updateIntPreference(QString name, int value);
int getIntPreference(QString name);
QString getStringPreference(QString name);
QString setStringPreference(QString name, QString value);
void getPreferences();
void updatePreferences();
QString getRgbFromColorName(const QString &colorName);
QStringList extractFieldsWithPrefix(const QStringList &fields, const QString &tableName, const QString &alias);
QString extractCurrentQuery(const QString &text, int cursorPos);
bool connectMySQL(const QString selectedHost, QObject *parent);
#endif // FUNCTIONS_H
