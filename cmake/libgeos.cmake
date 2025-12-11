# 设置第三方库的路径
set(3RD_GEOS_DIR "${3RD_PATH}/geos-3.11.2" CACHE PATH "geos install path")
# 设置第三方库的路径
if (MSVC)
    # 包含头文件目录
    include_directories(${3RD_GEOS_DIR}/include)
    # 链接库目录
    link_directories("${3RD_GEOS_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
else()
    # 包含头文件目录
    include_directories(${3RD_GEOS_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    # 链接库目录
    link_directories("${3RD_GEOS_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    # 设置库名称
endif ()
# 设置库名称
set(libgeos geos)

