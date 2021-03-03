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
#include "semesterloader.h"

template<typename T>
concept GetResultCallback = requires(T function, QSharedPointer<Client> client){
        function(client);
};

void getResult(QString& backendUrl, QString& authUrl, QString& name, QString& password, GetResultCallback auto callback){
    if(backendUrl == "" || authUrl == "" || name == "" || password == ""){
        qDebug() << "ERROR: missing inputs";
        exit(1);
    }

    ConnectionManager* cm = new ConnectionManager( QUrl(authUrl), QUrl(backendUrl) );
    cm->connect(cm, &ConnectionManager::loginSuccess, [&callback](QSharedPointer<Client> client){
        callback(client);
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

    QCommandLineOption listPlansOption("list-plans",
            "Lists all plans of a semester. You need to specify the semester option.");
    parser.addOption(listPlansOption);

    QCommandLineOption listSemestersOption("list-semesters",
            "Lists all semesters.");
    parser.addOption(listSemestersOption);

    QCommandLineOption downloadPlanOption("download-plan",
            "Download a plan. You need to specify the semester and the plan option. You can specify the type option to select csv or json. You can specify the path option to set the path where the files will be put.");
    parser.addOption(downloadPlanOption);

    QCommandLineOption uploadPlanOption("upload-plan",
            "Upload a plan. You need to specify the semester, the type and the plan option. You can specify the path option to select the files, that will be uploaded.");
    parser.addOption(uploadPlanOption);

    QCommandLineOption createSemesterOption("create-semester",
            "Create a new semester. You have to specify the semester option");
    parser.addOption(createSemesterOption);


    QCommandLineOption semesterOption("semester",
            "Select the name of the affected semester.", "semester");
    parser.addOption(semesterOption);

    QCommandLineOption planOption("plan",
            "Select the name of the affected plan.", "plan");
    parser.addOption(planOption);

    QCommandLineOption pathOption("path",
            "Select the path of the files to up or download.", "path", ".");
    parser.addOption(pathOption);

    QCommandLineOption typeOption("type",
            "Select the file format. Valid options are json and csv", "type", "csv");
    parser.addOption(typeOption);

    parser.process(a.arguments());

    QString authUrl = parser.value(authUrlOption);
    QString backendUrl = parser.value(backendUrlOption);
    QString schedulerUrl = parser.value(schedulerUrlOption);
    QString name = parser.value(nameOption);
    QString password = parser.value(passwordOption);

    QString semester = parser.value(semesterOption);
    QString plan = parser.value(planOption);
    QString path = parser.value(pathOption);
    QString type = parser.value(typeOption);

    if(type != "csv" && type != "json"){
        qDebug() << "ERROR: Type needs to be 'json' or 'csv'";
        exit(1);
    }

    QDir directory(path);

    if(!directory.exists()){
        qDebug() << "ERROR: Path needs to be a valid directory";
        exit(1);
    }

    int commands = 0;
    QString command;
    if(parser.isSet("list-plans")){
        commands++;
        command = "list-plans";
    }
    if(parser.isSet("list-semesters")){
        commands++;
        command = "list-semesters";
    }
    if(parser.isSet("download-plan")){
        commands++;
        command = "download-plan";
    }
    if(parser.isSet("upload-plan")){
        commands++;
        command = "upload-plan";
    }
    if(parser.isSet("create-semester")){
        commands++;
        command = "create-semester";
    }
    if(commands == 0){
        qDebug() << "ERROR: You have specified no command, try --list-plans, --list-semesters, --download-plan, --upload-plan or --create-semester.";
        exit(1);
    }else if(commands > 1){
        qDebug() << "ERROR: You have specified more than one command, please use only one of --list-plans, --list-semesters, --download-plan, --upload-plan or --create-semester.";
        exit(1);
    }

    if(command == "list-plans" || command == "upload-plan" || command == "download-plan" || command == "create-semester"){
        if(semester == ""){
            qDebug() << "ERROR: You need to specify a semester for " << command << ".";
            exit(1);
        }
    }
    if(command == "upload-plan" || command == "download-plan"){
        if(plan == ""){
            qDebug() << "ERROR: You need to specify a plan for " << command << ".";
            exit(1);
        }
    }
    if(command == "upload-plan" || command == "download-plan"){
        if(path == ""){
            qDebug() << "ERROR: You need to specify a path for " << command << ".";
            exit(1);
        }
    }
    if(command == "upload-plan" || command == "download-plan"){
        if(type == ""){
            qDebug() << "ERROR: You need to specify a type for " << command << ".";
            exit(1);
        }
    }

    getResult(backendUrl, authUrl, name, password, [command, semester, plan, directory, type](QSharedPointer<Client> client){
        qDebug() << "Logged in";
        SemesterLoader loader(client);
        if(command == "list-plans"){
            loader.listPlans(semester);
        }
        if(command == "list-semesters"){
            loader.listSemesters();
        }
        if(command == "upload-plan"){
            //loader.uploadPlan(directory, semester, plan, (type=="csv"));
            qDebug() << "upload-plan not yet supported.";
            exit(1);
        }
        if(command == "download-plan"){
            loader.downloadPlan(directory, semester, plan, (type=="csv"));
        }
        if(command == "create-semester"){
            //loader.createSemester(semester);
            qDebug() << "create-semester not yet supported.";
            exit(1);
        }
    });

    return a.exec();
}
