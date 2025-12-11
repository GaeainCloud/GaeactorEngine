QT = core

CONFIG += c++17

TARGET = gaeactor-transmit
DEFINES += GAEACTOR_TRANSMIT_LIBRARY
CONFIG += force_debug_info

TEMPLATE = lib
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$_PRO_FILE_PWD_/../projects/dynamic-lib-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-fastrtps.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-demservice-standalone.pri)


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    demservice/DemserviceStandalone.h \
    demservice/srtmHgtReader.h \
    gaeactor_transmit_define.h \
    gaeactor_transmit_global.h \
    gaeactor_transmit_interface.h \



SOURCES += \
    demservice/DemserviceStandalone.cpp \
    demservice/srtmHgtReader.c \
    gaeactor_transmit_interface.cpp \




copyDynamicLibrary(gaeactor-transmit)
