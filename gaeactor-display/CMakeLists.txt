# LaViCDesktop 配置
project(LaViCDesktop VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)

include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_environment.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_presentation.cmake)
include(${CMAKE_DIR}/sdk_crashservice.cmake)



find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Qml REQUIRED)
find_package(Qt5 COMPONENTS Quickwidgets)
find_package(Qt5 COMPONENTS Quick REQUIRED)
find_package(Qt5 COMPONENTS AxContainer)

set(QT_INCLUDES
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Qml_INCLUDE_DIRS}
    ${Qt5QuickWidgets_INCLUDE_DIRS}
    ${Qt5Quick_INCLUDE_DIRS}
    ${Qt5AxContainer_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS})

include_directories(${QT_INCLUDES})
set(QT_LIBRARIES
    Qt5::Widgets
    Qt5::Network
    Qt5::Qml
    Qt5::QuickWidgets
    Qt5::Quick
    Qt5::AxContainer
    Qt5::Gui)

# file(GLOB_RECURSE  SRCS_CPP ./*.cpp ./*.c ./*.cxx)
# file(GLOB_RECURSE  SRCS_H ./*.h ./*.hpp)

set(SRCS_H 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/configmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/eventdriver/eventdriver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/global_variables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/ganttwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qabstractrangemodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttdata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttmodel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttmodelitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttmodellist.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qrangeview.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialcheckable.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialcheckable_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialcheckable_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialoverlaywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialripple.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialrippleoverlay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialstatetransition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialstatetransitionevent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialstyle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialstyle_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialtheme.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialtheme_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcheckbox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcheckbox_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcircularprogress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcircularprogress_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcircularprogress_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcombobox.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcombobox_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialflatbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialflatbutton_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialflatbutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialhboxlayoutcomponentgroup.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialhboxlayoutcomponentgroup_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialiconbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialiconbutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmateriallineedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmateriallineedit_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialprogress.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialprogress_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialprogress_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialradiobutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialradiobutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialraisedbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialraisedbutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialscrollbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialscrollbar_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialscrollbar_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialslider.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialslider_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialslider_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialsnackbar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialsnackbar_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialsnackbar_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialstatusbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialstatusbutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtext_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtextbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtextbutton_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtoggle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtoggle_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtoggle_p.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/agenteditpanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/agenteditwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/agentslistwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/instagentslistwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/pathitemwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/pathpanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/playwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/replayitemwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/replaypanel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/replaywidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/runningwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/settingswidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/sostepslistwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/trackingwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/treectrl.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/datamanager/datamanager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/gaeactormanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/httpclient/httpclient.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/mapeditwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/mapwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/runtimeeditwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/runtimelistwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/widgetmanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/HttpServerComponent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/ErrorHandler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/SwaggerComponent.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/controller/StaticController.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/dto/StatusDto.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/handler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/controller/AgentController.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/dto/AgentDto.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/dto/PageDto.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/service/AgentService.hpp)

set(SRCS_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/configmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/eventdriver/eventdriver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/global_variables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/ganttwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qabstractrangemodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttmodel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttmodelitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qganttmodellist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qgantt/qrangeview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialcheckable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialcheckable_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialoverlaywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialripple.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialrippleoverlay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialstatetransition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialstyle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/lib/qtmaterialtheme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcheckbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcircularprogress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcircularprogress_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialcombobox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialflatbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialflatbutton_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialhboxlayoutcomponentgroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialiconbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmateriallineedit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialprogress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialprogress_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialradiobutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialraisedbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialscrollbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialscrollbar_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialslider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialslider_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialsnackbar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialsnackbar_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialstatusbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtextbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtoggle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/Component/qtmaterialtoggle_internal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/agenteditpanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/agenteditwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/agentslistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/instagentslistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/pathitemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/pathpanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/playwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/replayitemwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/replaypanel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/replaywidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/runningwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/settingswidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/sostepslistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/trackingwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/treectrl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/datamanager/datamanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/gaeactormanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/httpclient/httpclient.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/glad.c
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/mapwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/runtimeeditwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/runtimelistwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/mapeditwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uiwidget/widgetmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/ErrorHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver/service/AgentService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/httpserver/httpserver.cpp)
    
file(GLOB SRCS_UI_FORMS *.ui)

file(GLOB RC_FILES ./*.qrc)

file(GLOB SRCS_TRANSLATIONS *.ts)

add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP} ${SRCS_UI_FORMS} ${SRCS_TRANSLATIONS} ${RC_FILES})


target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_utils_location}                        
                        ${sdk_gaeactor_transmit}
                        ${sdk_gaeactor_comm}
                        ${sdk_gaeactor_environment}
                        ${sdk_loggingservice}
                        ${sdk_crashservice}
                        ${sdk_gaeactor_presentation})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE USING_QT_OPENGL_FUNCTION USING_SHADER)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 