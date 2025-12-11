QT       += core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console

TARGET = gaeactor-benchmark
CONFIG += force_debug_info
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-engine.pri)

win32: {
LIBS += -lAdvapi32
}

SOURCES += \
    main.cpp


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


copyApp(gaeactor-benchmark)
