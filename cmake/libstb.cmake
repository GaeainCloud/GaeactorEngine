# 设置第三方库的路径
set(3RD_STB_DIR "${3RD_PATH}/stb" CACHE PATH "stb install path")
# 包含头文件目录
include_directories(${3RD_STB_DIR})
# 链接库目录
# 设置库名称