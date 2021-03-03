QT -= gui
QT += websockets core
CONFIG += c++11 console network
CONFIG -= app_bundle

include($$PWD/libs/pruefungsplaner-datamodel/pruefungsplaner-datamodel.pri)
include($$PWD/libs/pruefungsplaner-auth/client/client.pri)
INCLUDEPATH += $$PWD/libs/cpptoml/include

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/client.cpp \
    src/connectionmanager.cpp \
    src/main.cpp

HEADERS += \
    src/client.h \
    src/connectionmanager.h

test{
    include($$PWD/libs/gtest/gtest_dependency.pri)

    QT += testlib
    TEMPLATE = app
    TARGET = pruefungsplaner-cli-tests
    INCLUDEPATH *= $$PWD/src

    CONFIG *= thread
    LIBS *= -lgtest -lgtest_main

    SOURCES -= src/main.cpp
    SOURCES += tests/clitests.cpp
}
else{
    TEMPLATE = app
    TARGET = pruefungsplaner-cli
}

unix{
    # Install executable
    target.path = /usr/bin
}

!isEmpty(target.path): INSTALLS += target
