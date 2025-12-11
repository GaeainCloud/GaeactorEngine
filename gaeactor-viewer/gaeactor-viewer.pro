QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

CONFIG += console

TARGET = gaeactor-viewer
CONFIG += force_debug_info
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += $$PWD/../component
INCLUDEPATH += $$PWD/../gaeactor-agent-cores
INCLUDEPATH += $$PWD/../gaeactor-agent-sensors

DEFINES += BUILD_WITH_EASY_PROFILER

include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)

include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-hiredis103.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-redis++1_3_12.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-libevent.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-iceoryx.pri)
include($$_PRO_FILE_PWD_/lib/lib.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libuv.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libzlib.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuWS.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libcurl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libWebsocketpp.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-ecal_5.12.4.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-onetbb_2021.13.0.pri)
SOURCES += \
    gaeactormanager.cpp \
    main.cpp \
    mainwindow.cpp \
    $$_PRO_FILE_PWD_/src/QGVWidgetTools.cpp \
    $$_PRO_FILE_PWD_/src/ellipseitem.cpp \
    $$_PRO_FILE_PWD_/src/pieitem.cpp \
    $$_PRO_FILE_PWD_/src/polygonitem.cpp \
    $$_PRO_FILE_PWD_/src/rectangleitem.cpp \
    src/baseitem.cpp \
    src/imageitem.cpp \
    src/lineitem.cpp \
    src/listviewitem.cpp

HEADERS += \
    gaeactormanager.h \
    mainwindow.h\
    $$_PRO_FILE_PWD_/src/QGVWidgetTools.h \
    $$_PRO_FILE_PWD_/src/ellipseitem.h \
    $$_PRO_FILE_PWD_/src/pieitem.h \
    $$_PRO_FILE_PWD_/src/polygonitem.h \
    $$_PRO_FILE_PWD_/src/rectangleitem.h \
    src/baseitem.h \
    src/imageitem.h \
    src/lineitem.h \
    src/listviewitem.h \
    testdata.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


copyApp(gaeactor-viewer)
