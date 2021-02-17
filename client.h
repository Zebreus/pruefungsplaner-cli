#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QTimer>

class Client : public QObject
{
    Q_OBJECT

    QUrl url;
    QWebSocket webSocket;
    QTimer timer;
    bool planning;

public:
    explicit Client(const QUrl &url, QObject *parent = nullptr);
    void updatePlan();
    void save(QJsonValue semesters);
    void login(const QString &token);
    void open();
signals:
    void gotResult(QJsonValue result);
    void socketError();
    void loginFailed();
    void loginSuccess();
    void savingFailed();
    void savingSuccess();

private Q_SLOTS:
    void onError(QAbstractSocket::SocketError onError);
    void onConnected();
    void onTextMessageReceived(QString message);
};

#endif // CLIENT_H
