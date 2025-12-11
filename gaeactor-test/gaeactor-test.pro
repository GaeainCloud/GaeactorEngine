QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console

TARGET = gaeactor-test
CONFIG += force_debug_info
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += $$PWD/../component
INCLUDEPATH += $$PWD/../gaeactor-agent-cores
INCLUDEPATH += $$PWD/../gaeactor-agent-sensors

include($$_PRO_FILE_PWD_/../projects/sdk-hiredis103.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-redis++1_3_12.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libevent.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)
include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-ulid.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-iceoryx.pri)
include($$_PRO_FILE_PWD_/../gaeactor-viewer/lib/lib.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libuv.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libzlib.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuWS.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libcurl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libWebsocketpp.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-onetbb_2021.13.0.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)
win32: {
LIBS += -lAdvapi32
}

SOURCES += \
    bspline.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    bspline.h \
    mainwindow.h \
    testdata.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


copyApp(gaeactor-test)
