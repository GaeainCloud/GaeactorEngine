QT       += core gui opengl websockets network quickwidgets axcontainer charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

CONFIG += console

TARGET = LaViCDesktop
CONFIG += force_debug_info

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

RESOURCES += qml.qrc

QML_IMPORT_PATH =

QML_DESIGNER_IMPORT_PATH =

DEFINES += USING_QT_OPENGL_FUNCTION
DEFINES += USING_SHADER


DEFINES += BUILD_WITH_EASY_PROFILER

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include($$_PRO_FILE_PWD_/../projects/gui-app-install.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-ulid.pri)
#include($$_PRO_FILE_PWD_/../projects/sdk-sole_ulid.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils-location.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-transmit.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-environment.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-comm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-presentation.pri)


include($$_PRO_FILE_PWD_/../projects/sdk-libassimp.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libfreeimage.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libfreetype2.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libstb.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libgl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglfw.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libglad.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libkhr.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libfbx.pri)


include($$_PRO_FILE_PWD_/../projects/sdk-ecal_5.12.4.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuv.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libzlib.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuWS.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libopenssl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libcurl.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libqtxlsx.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-loggingservice.pri)

win32: {
include($$_PRO_FILE_PWD_/../projects/sdk-google_breakpad.pri)
}
include($$_PRO_FILE_PWD_/../projects/sdk-crashservice.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-libWebsocketpp.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-oatpp-1.3.0.pri)

include($$_PRO_FILE_PWD_/../projects/sdk-sqlite-orm.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-onetbb_2021.13.0.pri)

win32: {
LIBS += -lws2_32 -lUser32 -lgdi32 -ladvapi32 -lIphlpapi -lShell32
}

QML_IMPORT_PATH += $$_PRO_FILE_PWD_/qml

SOURCES += \
    src/components/configmanager.cpp \
    src/components/eventdriver/eventdriver.cpp \
    src/components/global_variables.cpp \
    src/qgantt/ganttwidget.cpp \
    src/qgantt/qabstractrangemodel.cpp \
    src/qgantt/qganttdata.cpp \
    src/qgantt/qganttmodel.cpp \
    src/qgantt/qganttmodelitem.cpp \
    src/qgantt/qganttmodellist.cpp \
    src/qgantt/qrangeview.cpp \
    src/uiwidget/Component/lib/qtmaterialcheckable.cpp \
    src/uiwidget/Component/lib/qtmaterialcheckable_internal.cpp \
    src/uiwidget/Component/lib/qtmaterialoverlaywidget.cpp \
    src/uiwidget/Component/lib/qtmaterialripple.cpp \
    src/uiwidget/Component/lib/qtmaterialrippleoverlay.cpp \
    src/uiwidget/Component/lib/qtmaterialstatetransition.cpp \
    src/uiwidget/Component/lib/qtmaterialstyle.cpp \
    src/uiwidget/Component/lib/qtmaterialtheme.cpp \
    src/uiwidget/Component/qtmaterialcheckbox.cpp \
    src/uiwidget/Component/qtmaterialcircularprogress.cpp \
    src/uiwidget/Component/qtmaterialcircularprogress_internal.cpp \
    src/uiwidget/Component/qtmaterialcombobox.cpp \
    src/uiwidget/Component/qtmaterialflatbutton.cpp \
    src/uiwidget/Component/qtmaterialflatbutton_internal.cpp \
    src/uiwidget/Component/qtmaterialhboxlayoutcomponentgroup.cpp \
    src/uiwidget/Component/qtmaterialiconbutton.cpp \
    src/uiwidget/Component/qtmateriallineedit.cpp \
    src/uiwidget/Component/qtmaterialprogress.cpp \
    src/uiwidget/Component/qtmaterialprogress_internal.cpp \
    src/uiwidget/Component/qtmaterialradiobutton.cpp \
    src/uiwidget/Component/qtmaterialraisedbutton.cpp \
    src/uiwidget/Component/qtmaterialscrollbar.cpp \
    src/uiwidget/Component/qtmaterialscrollbar_internal.cpp \
    src/uiwidget/Component/qtmaterialslider.cpp \
    src/uiwidget/Component/qtmaterialslider_internal.cpp \
    src/uiwidget/Component/qtmaterialsnackbar.cpp \
    src/uiwidget/Component/qtmaterialsnackbar_internal.cpp \
    src/uiwidget/Component/qtmaterialstatusbutton.cpp \
    src/uiwidget/Component/qtmaterialtext.cpp \
    src/uiwidget/Component/qtmaterialtextbutton.cpp \
    src/uiwidget/Component/qtmaterialtoggle.cpp \
    src/uiwidget/Component/qtmaterialtoggle_internal.cpp \
    src/uiwidget/agenteditpanel.cpp \
    src/uiwidget/agenteditwidget.cpp \
    src/uiwidget/agentslistwidget.cpp \
    src/uiwidget/instagentslistwidget.cpp \
    src/uiwidget/pathitemwidget.cpp \
    src/uiwidget/pathpanel.cpp \
    src/uiwidget/playwidget.cpp \
    src/uiwidget/replayitemwidget.cpp \
    src/uiwidget/replaypanel.cpp \
    src/uiwidget/replaywidget.cpp \
    src/uiwidget/runningwidget.cpp \
    src/uiwidget/settingswidget.cpp \
    src/uiwidget/sostepslistwidget.cpp \
    src/uiwidget/trackingwidget.cpp \
    src/uiwidget/treectrl.cpp \
    src/datamanager/datamanager.cpp \
    src/components/gaeactormanager.cpp \
    src/components/httpclient/httpclient.cpp \
    src/components/glad.c \
    main.cpp \
    src/uiwidget/mapwidget.cpp \
    src/uiwidget/runtimeeditwidget.cpp \
    src/uiwidget/runtimelistwidget.cpp \
    src/uiwidget/mapeditwidget.cpp \
    src/uiwidget/widgetmanager.cpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/ErrorHandler.cpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver/service/AgentService.cpp \
    $$_PRO_FILE_PWD_/src/httpserver/httpserver.cpp \

HEADERS += \
    src/components/configmanager.h \
    src/components/eventdriver/eventdriver.h \
    src/components/global_variables.h \
    src/qgantt/ganttwidget.h \
    src/qgantt/qabstractrangemodel.h \
    src/qgantt/qganttdata.h \
    src/qgantt/qganttmodel.h \
    src/qgantt/qganttmodelitem.h \
    src/qgantt/qganttmodellist.h \
    src/qgantt/qrangeview.h \
    src/uiwidget/Component/lib/qtmaterialcheckable.h \
    src/uiwidget/Component/lib/qtmaterialcheckable_internal.h \
    src/uiwidget/Component/lib/qtmaterialcheckable_p.h \
    src/uiwidget/Component/lib/qtmaterialoverlaywidget.h \
    src/uiwidget/Component/lib/qtmaterialripple.h \
    src/uiwidget/Component/lib/qtmaterialrippleoverlay.h \
    src/uiwidget/Component/lib/qtmaterialstatetransition.h \
    src/uiwidget/Component/lib/qtmaterialstatetransitionevent.h \
    src/uiwidget/Component/lib/qtmaterialstyle.h \
    src/uiwidget/Component/lib/qtmaterialstyle_p.h \
    src/uiwidget/Component/lib/qtmaterialtheme.h \
    src/uiwidget/Component/lib/qtmaterialtheme_p.h \
    src/uiwidget/Component/qtmaterialcheckbox.h \
    src/uiwidget/Component/qtmaterialcheckbox_p.h \
    src/uiwidget/Component/qtmaterialcircularprogress.h \
    src/uiwidget/Component/qtmaterialcircularprogress_internal.h \
    src/uiwidget/Component/qtmaterialcircularprogress_p.h \
    src/uiwidget/Component/qtmaterialcombobox.h \
    src/uiwidget/Component/qtmaterialcombobox_p.h \
    src/uiwidget/Component/qtmaterialflatbutton.h \
    src/uiwidget/Component/qtmaterialflatbutton_internal.h \
    src/uiwidget/Component/qtmaterialflatbutton_p.h \
    src/uiwidget/Component/qtmaterialhboxlayoutcomponentgroup.h \
    src/uiwidget/Component/qtmaterialhboxlayoutcomponentgroup_p.h \
    src/uiwidget/Component/qtmaterialiconbutton.h \
    src/uiwidget/Component/qtmaterialiconbutton_p.h \
    src/uiwidget/Component/qtmateriallineedit.h \
    src/uiwidget/Component/qtmateriallineedit_p.h \
    src/uiwidget/Component/qtmaterialprogress.h \
    src/uiwidget/Component/qtmaterialprogress_internal.h \
    src/uiwidget/Component/qtmaterialprogress_p.h \
    src/uiwidget/Component/qtmaterialradiobutton.h \
    src/uiwidget/Component/qtmaterialradiobutton_p.h \
    src/uiwidget/Component/qtmaterialraisedbutton.h \
    src/uiwidget/Component/qtmaterialraisedbutton_p.h \
    src/uiwidget/Component/qtmaterialscrollbar.h \
    src/uiwidget/Component/qtmaterialscrollbar_internal.h \
    src/uiwidget/Component/qtmaterialscrollbar_p.h \
    src/uiwidget/Component/qtmaterialslider.h \
    src/uiwidget/Component/qtmaterialslider_internal.h \
    src/uiwidget/Component/qtmaterialslider_p.h \
    src/uiwidget/Component/qtmaterialsnackbar.h \
    src/uiwidget/Component/qtmaterialsnackbar_internal.h \
    src/uiwidget/Component/qtmaterialsnackbar_p.h \
    src/uiwidget/Component/qtmaterialstatusbutton.h \
    src/uiwidget/Component/qtmaterialstatusbutton_p.h \
    src/uiwidget/Component/qtmaterialtext.h \
    src/uiwidget/Component/qtmaterialtext_p.h \
    src/uiwidget/Component/qtmaterialtextbutton.h \
    src/uiwidget/Component/qtmaterialtextbutton_p.h \
    src/uiwidget/Component/qtmaterialtoggle.h \
    src/uiwidget/Component/qtmaterialtoggle_internal.h \
    src/uiwidget/Component/qtmaterialtoggle_p.h \
    src/uiwidget/agenteditpanel.h \
    src/uiwidget/agenteditwidget.h \
    src/uiwidget/agentslistwidget.h \
    src/uiwidget/instagentslistwidget.h \
    src/uiwidget/pathitemwidget.h \
    src/uiwidget/pathpanel.h \
    src/uiwidget/playwidget.h \
    src/uiwidget/replayitemwidget.h \
    src/uiwidget/replaypanel.h \
    src/uiwidget/replaywidget.h \
    src/uiwidget/runningwidget.h \
    src/uiwidget/settingswidget.h \
    src/uiwidget/sostepslistwidget.h \
    src/uiwidget/trackingwidget.h \
    src/uiwidget/treectrl.h \
    src/datamanager/datamanager.hpp \
    src/components/gaeactormanager.h \
    src/components/httpclient/ApiModels.hpp \
    src/components/httpclient/httpclient.hpp \
    src/uiwidget/mapeditwidget.h \
    src/uiwidget/mapwidget.h \
    src/uiwidget/runtimeeditwidget.h \
    src/uiwidget/runtimelistwidget.h \
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

TRANSLATIONS = gaeactor_display_cn.ts \
               gaeactor_display_us.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
copyApp(gaeactor-display)
