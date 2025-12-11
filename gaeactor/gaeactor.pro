QT = core

CONFIG += c++17 cmdline
#CONFIG += c++17 console

TARGET = gaeactor
CONFIG += force_debug_info
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += BUILD_WITH_EASY_PROFILER


DEFINES += USING_GAEACTOR_EXPORT_LIB

contains(DEFINES, USING_GAEACTOR_EXPORT_LIB) {
    DEFINES += GAEACTOR_LIBRARY
    TEMPLATE = lib
    CONFIG -= app_bundle
} else{

}





INCLUDEPATH += $$PWD/../component
INCLUDEPATH += $$PWD/../gaeactor-agent-cores
INCLUDEPATH += $$PWD/../gaeactor-agent-sensors

include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
include($$_PRO_FILE_PWD_/../projects/dynamic-lib-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-agent-cores.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-agent-sensors.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-interactions.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-auditions.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-event-engine.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-iceoryx.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-hiredis103.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libevent.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-environment.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)

SOURCES += \
    gaeactortransmitmanager.cpp \
    main.cpp

HEADERS += \
    gaeactortransmitmanager.h \
    gaeactor_global.h


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target




contains(DEFINES, USING_GAEACTOR_EXPORT_LIB) {
    copyDynamicLibrary(gaeactor)
} else {
    copyApp(gaeactor)
}
