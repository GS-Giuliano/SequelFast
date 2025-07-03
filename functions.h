#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QDebug>

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


void openPreferences();

// Conections (MySQL)
QJsonObject getConnection(QString selectedHost);
bool addConnection(QString name);
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

#endif // FUNCTIONS_H
