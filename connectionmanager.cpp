#include "connectionmanager.h"

ConnectionManager::ConnectionManager(const QUrl &securityProviderUrl, const QUrl& planerBackendUrl, QObject *parent):
    QObject(parent), securityProviderUrl(securityProviderUrl), planerBackendUrl(planerBackendUrl)
{
    planerClient = QSharedPointer<Client>(new Client(planerBackendUrl));
    connect(planerClient.data(), &Client::loginFailed, this, &ConnectionManager::planerLoginFailed);
    connect(planerClient.data(), &Client::loginSuccess, this, &ConnectionManager::planerLoginSuccess);
    connect(planerClient.data(), &Client::socketError, this, &ConnectionManager::planerSocketError);
    connect(&providerClient, &pruefungsplanerAuth::Client::gotToken, this, &ConnectionManager::gotToken);
    connect(&providerClient, &pruefungsplanerAuth::Client::error, this, &ConnectionManager::providerError);
    //connect(&client, &securityprovider::Client::onConnected, [](){qDebug() << "connected";});
    providerClient.open(securityProviderUrl);
    planerClient->open();
}

QString ConnectionManager::getUsername() const
{
    return m_username;
}

void ConnectionManager::login(QString username, QString password)
{
    qDebug() << "Login";
    QList<QString> claims{"pruefungsplanerRead", "pruefungsplanerWrite"};
    QString audience = "pruefungsplaner-backend";
    providerClient.getToken(username, password, claims, audience);
}

void ConnectionManager::setUsername(QString username)
{
    if (m_username == username)
        return;

    m_username = username;
    emit usernameChanged(m_username);
}

void ConnectionManager::gotToken(QString token)
{
    //TODO connect to service
    qDebug() << "gotToken";
    planerClient->login(token);
}

void ConnectionManager::providerError(pruefungsplanerAuth::Client::Error error)
{
    QString message;
    switch(error){
        case pruefungsplanerAuth::Client::Error::SOCKET_ERROR:
            //: Errormessage: auth server unreachable
            message = tr("Der Authentifizierungsserver ist gerade nicht erreichbar. Versuch es später nochmal.");
        break;
        case pruefungsplanerAuth::Client::Error::SERVER_ERROR:
            //: Errormessage: auth server error
            message = tr("Der Authentifizierungsserver hat ein Problem. Am besten schaust du mal nach ihm.");
        break;
        case pruefungsplanerAuth::Client::Error::INVALID_RESPONSE:
            //: Errormessage: auth server invalid response
            message = tr("Der Authentifizierungsserver hat mit einer ungültige Nachricht geantwortet. Am besten schaust du mal nach ihm.");
        break;
        case pruefungsplanerAuth::Client::Error::UNEXPECTED_MESSAGE:
            //: Errormessage: auth server unexpected message
            message = tr("Der Authentifizierungsserver sendet seltsame Nachrichten. Am besten schaust du mal nach ihm.");
    }
    qDebug() << "Provider Error : " << error;
    emit loginError(message);
}

void ConnectionManager::planerLoginSuccess()
{
    qDebug() << "Successful login";
    emit loginSuccess();
}

void ConnectionManager::planerLoginFailed()
{
    qDebug() << "Invalid login";
    //: Errormessage: invalid username or password
    emit loginError(tr("Der Nutzername und das Password scheinen nicht zu passen. Bitte überprüfe deine Angaben und versuche es nochmal."));
}

void ConnectionManager::planerSocketError()
{
    qDebug() << "Backend not available";
    //: Errormessage: backend not available
    emit loginError(tr("Der Pruefungsplanerserver ist gerade nicht erreichbar. Versuch es nachher nochmal."));
}
