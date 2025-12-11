
add_compile_definitions (QGV_EXPORT)

find_package(Qt5 COMPONENTS Gui REQUIRED)
find_package(Qt5 COMPONENTS Network REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS})

include_directories(${QT_INCLUDES})
set(QT_LIBRARIES
    Qt5::Gui
    Qt5::Network
    Qt5::Widgets)

# 设置第三方库的路径
set(3RD_QGEOVIEW_DIR "../gaeactor-viewer/lib" CACHE PATH "QGeoView install path")
# 包含头文件目录
include_directories(${3RD_QGEOVIEW_DIR}/include)
include_directories(${3RD_QGEOVIEW_DIR}/include/QGeoView)

file(GLOB  QGEOVIEW_SRCS_CPP ${3RD_QGEOVIEW_DIR}/src/*.cpp )
file(GLOB  QGEOVIEW_SRCS_H ${3RD_QGEOVIEW_DIR}/include/QGeoView/*.h )
