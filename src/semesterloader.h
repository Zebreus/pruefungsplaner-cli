#ifndef SEMESTERLOADER_H
#define SEMESTERLOADER_H

#include <QObject>
#include <semester.h>
#include <QDir>
#include "client.h"
#include "plancsvhelper.h"

template<typename T>
concept GetSemestersCallback = requires(T function, QList<Semester*> semesters){
    function(semesters);
};

class SemesterLoader : public QObject
{
    Q_OBJECT
    QSharedPointer<Client> client;
public:
    explicit SemesterLoader(QSharedPointer<Client> client, QObject *parent = nullptr);
    ~SemesterLoader();
    bool uploadPlan( QDir path, const QString& semester, const QString& plan, bool csv);
    bool downloadPlan(QDir path, const QString& semester, const QString& plan, bool csv);
    bool listSemesters();
    bool listPlans(const QString& semester);
    bool createSemester(const QString& semester);

private:
    void getSemesters(GetSemestersCallback auto callback);


signals:

};

#endif // SEMESTERLOADER_H
