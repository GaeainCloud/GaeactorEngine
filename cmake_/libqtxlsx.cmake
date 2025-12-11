# 设置第三方库的路径
set(3RD_QTXLSX_DIR "${3RD_PATH}/QtXlsx" CACHE PATH "QtXlsx install path")
# 包含头文件目录
include_directories(${3RD_QTXLSX_DIR}/include)
# 链接库目录
link_directories("${3RD_QTXLSX_DIR}/lib/$<CONFIG>")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libqtxlsx Qt5Xlsxd)
######################################################
else ()
######################################################
set(libqtxlsx Qt5Xlsx)
######################################################
endif ()