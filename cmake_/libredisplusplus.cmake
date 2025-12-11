# 设置第三方库的路径
set(3RD_REDISPLUSPLUS_DIR "${3RD_PATH}/redis-plus-plus-1.3.12" CACHE PATH "redisplusplus install path")
# 包含头文件目录
include_directories(${3RD_REDISPLUSPLUS_DIR}/include)
# 链接库目录
link_directories("${3RD_REDISPLUSPLUS_DIR}/lib/$<CONFIG>")
# 设置库名称
set(libredisplusplus redis++)