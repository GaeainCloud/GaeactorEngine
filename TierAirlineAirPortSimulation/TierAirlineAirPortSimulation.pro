QT       += core websockets network

CONFIG += c++17 console

TARGET = TierAirlineAirPortSimulation
CONFIG += force_debug_info
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

RESOURCES += qml.qrc

QML_IMPORT_PATH =

QML_DESIGNER_IMPORT_PATH =

#DEFINES += USING_QT_OPENGL_FUNCTION
#DEFINES += USING_SHADER


# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-environment.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libstb.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglad.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libkhr.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libfbx.pri)


include($$_PRO_FILE_PWD_/../projects/sdk-libuv.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libzlib.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuWS.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libcurl.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libqtxlsx.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-crashservice.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libWebsocketpp.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-oatpp-1.3.0.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-sqlite-orm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-hiredis103.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-redis++1_3_12.pri)


win32: {
include($$_PRO_FILE_PWD_/../projects/sdk-google_breakpad.pri)
LIBS += -lws2_32 -lUser32 -lgdi32 -ladvapi32 -lIphlpapi -lShell32
}
unix:{
include($$_PRO_FILE_PWD_/../projects/sdk-onetbb_2021.13.0.pri)
}
SOURCES += \
    src/components/configmanager.cpp \
    src/components/eventdriver/eventdriver.cpp \
    src/datamanager/datamanager.cpp \
    src/components/function.cpp \
    src/components/gaeactormanager.cpp \
    src/components/httpclient/httpclient.cpp \
    main.cpp \
    src/uiwidget/widgetmanager.cpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/ErrorHandler.cpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/service/AgentService.cpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver.cpp \

HEADERS += \
    src/components/configmanager.h \
    src/components/eventdriver/eventdriver.h \
    src/datamanager/datamanager.hpp \
    src/components/function.h \
    src/components/gaeactormanager.h \
    src/components/httpclient/ApiModels.hpp \
    src/components/httpclient/httpclient.hpp \
    src/uiwidget/widgetmanager.h \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/HttpServerComponent.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/ErrorHandler.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/SwaggerComponent.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/controller/StaticController.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/controller/UserController.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/dto/StatusDto.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/dto/UserDto.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/service/UserService.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/handler.h \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver.h \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/controller/AgentController.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/dto/AgentDto.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/dto/PageDto.hpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/service/AgentService.hpp

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
copyApp(TierAirlineAirPortSimulation)
