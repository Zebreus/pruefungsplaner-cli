#include "client.h"

Client::Client(const QUrl &url, QObject *parent):
    QObject(parent), url(url), planning(false)
{
    connect(&webSocket, &QWebSocket::connected, this, &Client::onConnected);
    connect(&timer, SIGNAL(timeout()), this, SLOT(requestProgress()));
    connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &Client::onError);
}

void Client::updatePlan()
{
    webSocket.sendTextMessage("{\"jsonrpc\":\"2.0\",\"method\":\"getSemesters\",\"id\":\"3\"}");
}

void Client::save(QJsonValue semesters)
{
    QString message = "{\"jsonrpc\":\"2.0\",\"method\":\"setSemesters\",\"params\":[";
    message.append(QJsonDocument(semesters.toArray()).toJson(QJsonDocument::Compact));
    message.append("],\"id\":\"9\"}");
    webSocket.sendTextMessage(message);
}

void Client::open()
{
    webSocket.open(QUrl(url));
}

void Client::onError(QAbstractSocket::SocketError)
{
    emit socketError();
}

void Client::login(const QString& token){
    QString loginRequest = QString("{\"jsonrpc\":\"2.0\",\"method\":\"login\",\"params\":[\"%1\"],\"id\":\"7\"}").arg(token);
    webSocket.sendTextMessage(loginRequest);
}

void Client::onConnected()
{
    qDebug() << "WebSocket connected";
    connect(&webSocket, &QWebSocket::textMessageReceived, this, &Client::onTextMessageReceived);
    connect(&webSocket, &QWebSocket::binaryMessageReceived, this, &Client::onTextMessageReceived);
}

void Client::onTextMessageReceived(QString message)
{
    QJsonDocument document = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject response = document.object();
    QJsonValue result = response.value("result");
    int id = response.value("id").toString().toInt();
    qDebug() << id << " got message:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    switch (id){
        case 3:
            emit gotResult(result);
            break;
    case 7:
            qDebug() << "Login result: " << result;
            if(result.toBool()){
                emit loginSuccess();
            }else{
                emit loginFailed();
            }
            break;
    case 9:
            qDebug() << "Saving result: " << result;
            if(result.toBool()){
                emit savingSuccess();
            }else{
                emit savingFailed();
            }
            break;
    default:
        qDebug() << "Unknown id";
    }

}
