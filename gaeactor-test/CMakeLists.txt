# gaeactor-test 配置
project(gaeactor-test VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/libgeoview.cmake)
include(${CMAKE_DIR}/lib3rd.cmake)

include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_transmit.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_comm.cmake)

# IF (MSVC)
#     set(ws232 ws2_32)
# ENDIF (MSVC)


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
    ${CMAKE_CURRENT_SOURCE_DIR}/bspline.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/testdata.h)

set(SRCS_CPP 
    ${CMAKE_CURRENT_SOURCE_DIR}/bspline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
    
file(GLOB SRCS_UI_FORMS *.ui)

add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP} ${SRCS_UI_FORMS} ${QGEOVIEW_SRCS_CPP} ${QGEOVIEW_SRCS_H})


target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES}
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_utils_location}                        
                        ${sdk_gaeactor_transmit}
                        ${sdk_gaeactor_comm})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 