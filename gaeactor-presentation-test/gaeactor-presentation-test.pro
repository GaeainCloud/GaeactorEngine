QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gaeactor-presentation-test
CONFIG += c++17

CONFIG += console
CONFIG += force_debug_info
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-presentation.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

copyApp(gaeactor-presentation-test)

