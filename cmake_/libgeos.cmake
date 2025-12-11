# 设置第三方库的路径
set(3RD_GEOS_DIR "${3RD_PATH}/geos-3.11.2" CACHE PATH "geos install path")
# 包含头文件目录
include_directories(${3RD_GEOS_DIR}/include)
# 链接库目录
link_directories("${3RD_GEOS_DIR}/lib/$<CONFIG>")
# 设置库名称
set(libgeos geos)