QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gaeactor-presentation-test3
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

include($$_PRO_FILE_PWD_/../projects/sdk-libassimp.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libgl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglfw.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglew_2.1.0.pri)



SOURCES += \
    CloudNode.cpp \
    TextureImage2D.cpp \
    ex/MeshHolder.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CloudNode.h \
    TextureImage2D.h \
    ex/MeshHolder.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

copyApp(gaeactor-presentation-test3)

