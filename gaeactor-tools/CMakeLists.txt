# gaeactor-tools 配置
project(gaeactor-tools VERSION 1.0 LANGUAGES CXX)


include(${CMAKE_DIR}/lib3rd.cmake)

include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)


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

file(GLOB_RECURSE  SRCS_CPP ./*.cpp ./*.c ./*.cxx)
file(GLOB_RECURSE  SRCS_H ./*.h ./*.hpp)    
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