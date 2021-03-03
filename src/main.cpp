#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QString>
#include <QDebug>
#include <QUrl>
#include <QSharedPointer>
#include "connectionmanager.h"
#include "client.h"
#include "semester.h"
#include "plan.h"
#include "plancsvhelper.h"
#include <QTemporaryDir>


void getResult(QString& backendUrl, QString& authUrl, QString& name, QString& password){
    if(backendUrl == "" || authUrl == "" || name == "" || password == ""){
        qDebug() << "ERROR: missing inputs";
        return;
    }

    ConnectionManager* cm = new ConnectionManager( QUrl(authUrl), QUrl(backendUrl) );
    cm->connect(cm, &ConnectionManager::loginSuccess, [](QSharedPointer<Client> client){
        qDebug() << "Logged in";
        QObject::connect(client.get(), &Client::gotResult, [](QJsonValue result){
            qDebug() << "Got result";
            if(result.isArray() && result.toArray()[0].isObject() && result.toArray()[0].toObject()["plans"].toArray()[0].isObject()){
                QJsonObject jsonPlan = result.toArray()[0].toObject()["plans"].toArray()[0].toObject();
                qDebug() << "json:" << jsonPlan;
                QSharedPointer<Plan> planPointer(new Plan());
                planPointer->fromJsonObject(jsonPlan);
                PlanCsvHelper csvHelper("./");
                csvHelper.writePlan(planPointer.get());

                qDebug() << "Got result" << planPointer->getWeeks();
            }else{
                qDebug() << "Got something unexpected";
            }
        });
        client->updatePlan();

    });

    cm->connect(cm, &ConnectionManager::loginError, [](const QString& message){
        qDebug() << "Failed: " << message;
    });
    cm->login(name, password);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Pruefungsplaner cli");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption authUrlOption("auth-url",
                                     "The url of the auth server. This should "
                                   "look like \"wss://0.0.0.0:443\".",
                                     "auth-url");
    parser.addOption(authUrlOption);

    QCommandLineOption backendUrlOption("backend-url",
                                        "The url of the backend server. This "
                                      "should look like \"wss://0.0.0.0:443\".",
                                        "backend-url");
    parser.addOption(backendUrlOption);

    QCommandLineOption schedulerUrlOption(
                "scheduler-url",
                "The url of the scheduler server. This should look like "
      "\"wss://0.0.0.0:443\".",
                "scheduler-url");
    parser.addOption(schedulerUrlOption);

    QCommandLineOption nameOption("name",
                                     "Your username",
                                     "name");
    parser.addOption(nameOption);

    QCommandLineOption passwordOption("password",
                                     "Your password",
                                     "password");
    parser.addOption(passwordOption);

    QCommandLineOption getResultOption("get-result",
            "Download the result into the workign directory");
    parser.addOption(getResultOption);

    parser.process(a.arguments());

    QString authUrl = parser.value(authUrlOption);
    QString backendUrl = parser.value(backendUrlOption);
    QString schedulerUrl = parser.value(schedulerUrlOption);
    QString name = parser.value(nameOption);
    QString password = parser.value(passwordOption);

    if(parser.isSet(getResultOption)){
        getResult(backendUrl, authUrl, name, password);
    }else{
        qDebug() << "ERROR: No operation set";
    }

    return a.exec();
}
