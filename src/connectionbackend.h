#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class ConnectionBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(QString port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString user READ user WRITE setUser NOTIFY userChanged)
    Q_PROPERTY(QString pass READ pass WRITE setPass NOTIFY passChanged)
    Q_PROPERTY(QString schema READ schema WRITE setSchema NOTIFY schemaChanged)
    Q_PROPERTY(QString sshHost READ sshHost WRITE setSshHost NOTIFY sshHostChanged)
    Q_PROPERTY(QString sshPort READ sshPort WRITE setSshPort NOTIFY sshPortChanged)
    Q_PROPERTY(QString sshUser READ sshUser WRITE setSshUser NOTIFY sshUserChanged)
    Q_PROPERTY(QString sshPass READ sshPass WRITE setSshPass NOTIFY sshPassChanged)
    Q_PROPERTY(QString sshKeyFile READ sshKeyFile WRITE setSshKeyFile NOTIFY sshKeyFileChanged)
    Q_PROPERTY(bool shared READ shared WRITE setShared NOTIFY sharedChanged)
    Q_PROPERTY(int colorIndex READ colorIndex WRITE setColorIndex NOTIFY colorIndexChanged)
    Q_PROPERTY(QVariantList colorList READ colorList CONSTANT)

public:
    explicit ConnectionBackend(QString selectedHost, QObject* parent = nullptr);

    QString name() const { return m_name; }
    void setName(const QString& v);

    QString host() const { return m_host; }
    void setHost(const QString& v);

    QString port() const { return m_port; }
    void setPort(const QString& v);

    QString user() const { return m_user; }
    void setUser(const QString& v);

    QString pass() const { return m_pass; }
    void setPass(const QString& v);

    QString schema() const { return m_schema; }
    void setSchema(const QString& v);

    QString sshHost() const { return m_sshHost; }
    void setSshHost(const QString& v);

    QString sshPort() const { return m_sshPort; }
    void setSshPort(const QString& v);

    QString sshUser() const { return m_sshUser; }
    void setSshUser(const QString& v);

    QString sshPass() const { return m_sshPass; }
    void setSshPass(const QString& v);

    QString sshKeyFile() const { return m_sshKeyFile; }
    void setSshKeyFile(const QString& v);

    bool shared() const { return m_shared; }
    void setShared(bool v);

    int colorIndex() const { return m_colorIndex; }
    void setColorIndex(int v);

    QVariantList colorList() const { return m_colorList; }

public slots:
    void onCancel();
    void onSave();
    void onRemove();
    void onConnect();

signals:
    void nameChanged();
    void hostChanged();
    void portChanged();
    void userChanged();
    void passChanged();
    void schemaChanged();
    void sshHostChanged();
    void sshPortChanged();
    void sshUserChanged();
    void sshPassChanged();
    void sshKeyFileChanged();
    void sharedChanged();
    void colorIndexChanged();

    void accepted();
    void rejected();
    void connectionFailed(QString message);
    void connectionSucceeded();

private:
    void saveConnection();

    QString m_originalName;
    QString m_colorName = "white";

    QString m_name;
    QString m_host;
    QString m_port;
    QString m_user;
    QString m_pass;
    QString m_schema;
    QString m_sshHost;
    QString m_sshPort;
    QString m_sshUser;
    QString m_sshPass;
    QString m_sshKeyFile;
    bool m_shared = false;
    int m_colorIndex = 0;
    QVariantList m_colorList;
};
