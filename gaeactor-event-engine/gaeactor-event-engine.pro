QT = core

CONFIG += c++17

TARGET = gaeactor-event-engine
DEFINES += GAEACTOR_EVENT_ENGINE_LIBRARY
CONFIG += force_debug_info

TEMPLATE = lib
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += BUILD_WITH_EASY_PROFILER

include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)
include($$_PRO_FILE_PWD_/../projects/dynamic-lib-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-environment.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-onetbb_2021.13.0.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    gaeactor_event_engine_define.h \
    gaeactor_event_engine_global.h \
    gaeactor_event_engine_interface.h \
    gaeactor_event_engine_processor.h

SOURCES += \
    gaeactor_event_engine_interface.cpp \
    gaeactor_event_engine_processor.cpp


copyDynamicLibrary(gaeactor-event-engine)
