# 设置第三方库的路径
set(3RD_GDAL_DIR "${3RD_PATH}/gdal-3.7.0" CACHE PATH "gdal install path")
# 设置第三方库的路径
if (MSVC)
    # 包含头文件目录
    include_directories(${3RD_GDAL_DIR}/include)
    # 链接库目录
    link_directories("${3RD_GDAL_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
else()
    # 包含头文件目录
    include_directories(${3RD_GDAL_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    # 链接库目录
    link_directories("${3RD_GDAL_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    # 设置库名称
endif ()
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libgdal gdald)
######################################################
else ()
######################################################
set(libgdal gdal)
######################################################
endif ()

