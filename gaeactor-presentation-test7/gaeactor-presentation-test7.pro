

DEFINES += USING_GUI_SHOW

contains(DEFINES, USING_GUI_SHOW){
QT       += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES += USING_QT_OPENGL_FUNCTION
DEFINES += USING_SHADER
TARGET = gaeactor-presentation-test7
}
else{
QT += core
CONFIG -= app_bundle
TARGET = gaeactor-presentation-test7
}


CONFIG += c++17 console
CONFIG += force_debug_info
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0




DEFINES += BUILD_WITH_EASY_PROFILER

include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-presentation.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-environment-ex.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-oatpp-1.3.0.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-sqlite-orm.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-onetbb_2021.13.0.pri)

win32: {
include($$_PRO_FILE_PWD_/../projects/sdk-google_breakpad.pri)
}
include($$_PRO_FILE_PWD_/../projects/sdk-crashservice.pri)

contains(DEFINES, USING_GUI_SHOW){
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-presentation.pri)
}
SOURCES += \
    ConcurrentHashMapManager.cpp \
    main.cpp \
    mainwindow.cpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/ErrorHandler.cpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/service/AgentService.cpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver.cpp \

HEADERS += \
    ConcurrentHashMapManager.h \
    mainwindow.h \
    $$_PRO_FILE_PWD_/httpserver/httpserver/HttpServerComponent.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/ErrorHandler.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/SwaggerComponent.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/controller/StaticController.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/controller/UserController.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/dto/StatusDto.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/dto/UserDto.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/service/UserService.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/handler.h \
    $$_PRO_FILE_PWD_/httpserver/httpserver.h \
    $$_PRO_FILE_PWD_/httpserver/httpserver/controller/AgentController.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/dto/AgentDto.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/dto/PageDto.hpp \
    $$_PRO_FILE_PWD_/httpserver/httpserver/service/AgentService.hpp \
    params_define.h

contains(DEFINES, USING_GUI_SHOW){
FORMS += \
    mainwindow.ui
}



win32: {
    INCLUDEPATH += $$_PRO_FILE_PWD_/../../bin/win64/sdk/src/SpaceIndexerArcadix

    CONFIG(debug, debug|release):
    {
        LIBS += -L$$_PRO_FILE_PWD_/../../bin/win64/sdk/lib/debug -lSpaceIndexerArcadixLib
    }

    CONFIG(release, debug|release):
    {
        LIBS += -L$$_PRO_FILE_PWD_/../../bin/win64/sdk/lib/release -lSpaceIndexerArcadixLib
    }
}


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

contains(DEFINES, USING_GUI_SHOW){
copyApp(gaeactor-presentation-test7)
}
else
{
copyApp(gaeactor-presentation-test7)
}

