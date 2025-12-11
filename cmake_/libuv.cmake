# 设置第三方库的路径
set(3RD_LIBUV_DIR "${3RD_PATH}/libuv-1.46.0" CACHE PATH "libuv install path")
# 包含头文件目录
include_directories(${3RD_LIBUV_DIR}/include)
# 链接库目录
link_directories("${3RD_LIBUV_DIR}/lib/$<CONFIG>")
# 设置库名称
set(libuv uv)