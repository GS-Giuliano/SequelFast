#pragma once

#include "globalvariablesmodel.h"

#include <QObject>
#include <QString>

// QML-facing backend for the Statistics screen: schema metadata as plain
// string properties, plus the filterable global-variables list model.
class StatisticsBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString schemaName READ schemaName CONSTANT)
    Q_PROPERTY(QString charSet READ charSet CONSTANT)
    Q_PROPERTY(QString collation READ collation CONSTANT)
    Q_PROPERTY(QString encryption READ encryption CONSTANT)
    Q_PROPERTY(QString size READ size CONSTANT)
    Q_PROPERTY(QString tableCount READ tableCount CONSTANT)
    Q_PROPERTY(GlobalVariablesModel* variablesModel READ variablesModel CONSTANT)

public:
    explicit StatisticsBackend(const QString& host, const QString& schema, QObject* parent = nullptr);

    QString title() const { return m_title; }
    QString schemaName() const { return m_schemaName; }
    QString charSet() const { return m_charSet; }
    QString collation() const { return m_collation; }
    QString encryption() const { return m_encryption; }
    QString size() const { return m_size; }
    QString tableCount() const { return m_tableCount; }
    GlobalVariablesModel* variablesModel() const { return m_variablesModel; }

public slots:
    void onClose();

signals:
    void closed();

private:
    QString m_title;
    QString m_schemaName;
    QString m_charSet;
    QString m_collation;
    QString m_encryption;
    QString m_size;
    QString m_tableCount;
    GlobalVariablesModel* m_variablesModel = nullptr;
};
