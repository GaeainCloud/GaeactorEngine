# gaeactor-viewer 配置
project(gaeactor-viewer VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/libgeoview.cmake)
include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)
include(${CMAKE_DIR}/sdk_loggingservice.cmake)



find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)

set(QT_INCLUDES
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS})

include_directories(${QT_INCLUDES})
set(QT_LIBRARIES
    Qt5::Widgets
    Qt5::Network
    Qt5::Gui)

set(SRCS_H 
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactormanager.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/QGVWidgetTools.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ellipseitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pieitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/polygonitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/rectangleitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/baseitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imageitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lineitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/listviewitem.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/testdata.h)

set(SRCS_CPP     
    ${CMAKE_CURRENT_SOURCE_DIR}/gaeactormanager.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}//src/QGVWidgetTools.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}//src/ellipseitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}//src/pieitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}//src/polygonitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}//src/rectangleitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/baseitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imageitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lineitem.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/listviewitem.cpp)
    
file(GLOB SRCS_UI_FORMS *.ui)

add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP} ${SRCS_UI_FORMS} ${QGEOVIEW_SRCS_CPP} ${QGEOVIEW_SRCS_H})


target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_utils_location}                        
                        ${sdk_gaeactor_transmit}
                        ${sdk_gaeactor_comm}
                        ${sdk_loggingservice})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 