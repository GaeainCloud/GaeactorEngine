# gaeactor-presentation-test5 配置
project(gaeactor-presentation-test5 VERSION 1.0 LANGUAGES CXX)

include(${CMAKE_DIR}/lib3rd.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_utils_location.cmake)
include(${CMAKE_DIR}/sdk_gaeactor_presentation.cmake)

IF (MSVC)
    set(ws232 ws2_32)
ENDIF (MSVC)


find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS OpenGL REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS})

include_directories(${QT_INCLUDES})
set(QT_LIBRARIES
    Qt5::Gui
    Qt5::OpenGL
    Qt5::Widgets)

file(GLOB_RECURSE  SRCS_CPP ./*.cpp ./*.c ./*.cxx)
file(GLOB_RECURSE  SRCS_H ./*.h ./*.hpp)
# file(GLOB RC_FILES ./*.qrc)
file(GLOB SRCS_UI_FORMS *.ui)

add_executable(${PROJECT_NAME} ${SRCS_H} ${SRCS_CPP} ${SRCS_UI_FORMS})


target_link_libraries(${PROJECT_NAME} 
                        ${QT_LIBRARIES} 
                        ${LIB3RD_LIBRARIES} 
                        ${sdk_gaeactor_utils}
                        ${sdk_gaeactor_utils_location}
                        ${sdk_gaeactor_presentation})

target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE USING_QT_OPENGL_FUNCTION USING_SHADER)

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib RUNTIME DESTINATION bin) 