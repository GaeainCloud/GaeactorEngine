#QT       += core gui network
QT       += core gui opengl websockets network quickwidgets axcontainer charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console

TARGET = gaeactor-tools
CONFIG += force_debug_info
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



include($$_PRO_FILE_PWD_/../projects/sdk-hiredis103.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libevent.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)
include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-ulid.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-iceoryx.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libglm.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libfreeimage.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libstb.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglad.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libcurl.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libqtxlsx.pri)

win32: {
LIBS += -lAdvapi32
}

SOURCES += \
    function.cpp \
    datamanager.cpp \
    main.cpp \
    mainwindow.cpp \
    taskthread.cpp

HEADERS += \
    datamanager.hpp \
    function.h \
    datamanager.h \
    mainwindow.h \
    taskthread.hpp \
    testdata.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


copyApp(gaeactor-tools)
