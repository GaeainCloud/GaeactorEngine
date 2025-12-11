# 设置第三方库的路径

set(3RD_ZLIB_DIR "${3RD_PATH}/zlib-1.3.0" CACHE PATH "zlib install path")
# 包含头文件目录
include_directories(${3RD_ZLIB_DIR}/include)
# 链接库目录
link_directories("${3RD_ZLIB_DIR}/lib/$<CONFIG>")
# 设置库名称
set(libzlib zlibwapi)