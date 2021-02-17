#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <pruefungsplaner-auth/client.h>
#include <QSharedPointer>
#include "client.h"

/*
 * The ConnectionManager should handle login and create a authorized connection to the server
*/
class ConnectionManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString username READ getUsername WRITE setUsername NOTIFY usernameChanged)

public:
    explicit ConnectionManager(const QUrl& securityProviderUrl, const QUrl& planerBackendUrl, QObject *parent = nullptr);

    QString getUsername() const;

signals:
    void loginSuccess(QSharedPointer<Client> client);
    void loginError(const QString& message);
    void usernameChanged(QString username);

public slots:
    void login(QString username, QString password);
    void setUsername(QString username);

private slots:
    void gotToken(QString token);
    void providerError(pruefungsplanerAuth::Client::Error error);
    void planerLoginSuccess();
    void planerLoginFailed();
    void planerSocketError();

private:
    QString token;
    QUrl securityProviderUrl;
    QUrl planerBackendUrl;
    pruefungsplanerAuth::Client providerClient;
    QSharedPointer<Client> planerClient;
    QString m_username;
};

#endif // CONNECTIONMANAGER_H
