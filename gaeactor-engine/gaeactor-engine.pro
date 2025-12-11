QT = core

CONFIG += c++17

TARGET = gaeactor-engine
DEFINES += GAEACTOR_ENGINE_LIBRARY
CONFIG += force_debug_info

TEMPLATE = lib
CONFIG -= app_bundle


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include($$_PRO_FILE_PWD_/../projects/dynamic-lib-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-heads.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor.pri)

win32: {
LIBS += -ladvapi32
}

SOURCES += \
    gaeactorInterface.cpp

HEADERS += \
    gaeactorInterface.h \
    gaeactor_engine_global.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


copyDynamicLibrary(gaeactor-engine)
