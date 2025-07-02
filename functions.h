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
QJsonObject getConnection(QString selectedHost);
bool addConnection(QString name);
bool deleteConnection(QString name);
bool connMysql(QWidget *parent, QString selectedHost);

#endif // FUNCTIONS_H
