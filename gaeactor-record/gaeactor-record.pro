QT       += core websockets network

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

CONFIG += console

TARGET = gaeactor-record
CONFIG += force_debug_info

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-redis++1_3_12.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuv.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libzlib.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuWS.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libcurl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-hiredis103.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-iceoryx.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libWebsocketpp.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-onetbb_2021.13.0.pri)

SOURCES += \
    src/configmanager.cpp \
    src/gaeactormanager.cpp \
    src/gzip.cpp \
    src/jsonprocessor.cpp \
    src/main.cpp

HEADERS += \
    src/configmanager.h \
    src/gaeactormanager.h \
    src/gzip.h \
    src/jsonprocessor.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
copyApp(gaeactor-record)
