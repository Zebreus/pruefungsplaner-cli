#include "semesterloader.h"

SemesterLoader::SemesterLoader(QSharedPointer<Client> client, QObject *parent):
    QObject(parent), client(client)
{

}

SemesterLoader::~SemesterLoader()
{
}

bool SemesterLoader::downloadPlan(QDir path, const QString &semesterName, const QString &planName, bool csv)
{
    getSemesters([semesterName, planName, path, csv](const QList<Semester*>& result){
        for(Semester* semester:result){
            if(semester->getName() == semesterName){
                for(Plan* plan:semester->getPlans()){
                    if(plan->getName() == planName){
                        if(csv){
                            PlanCsvHelper helper(path.absolutePath());
                            if(helper.writePlan(plan)){
                                exit(0);
                            }else{
                                qDebug() << "ERROR: Failed to write plan " << planName << " to " << path.absolutePath() << " as csv.";
                                exit(1);
                            }
                        }else{
                            QJsonDocument planDocument(plan->toJsonObject());
                            QFile file(path.absolutePath() + "/plan.json");
                            if (file.open(QFile::WriteOnly)) {
                                file.write(planDocument.toJson());
                                file.close();
                                exit(0);
                            }else{
                                qDebug() << "ERROR: Failed to write plan " << planName << " to " << path.absolutePath() << " as json document.";
                                exit(1);
                            }
                        }
                        exit(0);
                    }
                }
                qDebug() << "ERROR: Plan '" << planName << "' does not exist in semester " << semesterName << ".";
                exit(1);
            }
        }
        qDebug() << "ERROR: Semester '" << semesterName << "' does not exist.";
        exit(1);
    });
    return true;
}

bool SemesterLoader::listPlans(const QString &semesterName)
{
    getSemesters([semesterName](const QList<Semester*>& result){
        for(Semester* semester:result){
            if(semester->getName() == semesterName){
                QTextStream out(stdout);
                for(Plan* plan:semester->getPlans()){
                    out << plan->getName() << '\n';
                }
                out.flush();
                exit(0);
            }
        }
        qDebug() << "ERROR: Semester '" << semesterName << "' does not exist.";
        exit(1);
    });
    return true;
}

bool SemesterLoader::listSemesters()
{
    getSemesters([](const QList<Semester*>& result){
        QTextStream out(stdout);
        for(Semester* semester:result){
            out << semester->getName() << '\n';
        }
        out.flush();
        exit(0);
    });
    return true;
}

void SemesterLoader::getSemesters(GetSemestersCallback auto callback){
    qDebug() << "getSemesters called";
    connect(client.get(), &Client::gotResult, [callback](const QJsonValue& result){
        qDebug() << "getSemesters callback";
        if(result.isArray()){
            QJsonArray arr = result.toArray();
            QList<Semester*> semesters;
            for(QJsonValue val : arr){
                if(val.isObject()){
                    Semester* semester = new Semester();
                    semester->fromJsonObject(val.toObject());
                    semesters.push_back(semester);
                }else{
                    qDebug() << "ERROR: Backend did not return an array of objects";
                    exit(1);
                }
            }
            callback(semesters);
        }else{
            qDebug() << "ERROR: Backend did not return an array of semesters";
            exit(1);
        }
    });
    client->updatePlan();
}
