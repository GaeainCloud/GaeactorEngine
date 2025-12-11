# 设置第三方库的路径
set(3RD_KHR_DIR "${3RD_PATH}/khr" CACHE PATH "khr install path")
# 包含头文件目录
include_directories(${3RD_KHR_DIR}/include)
# 链接库目录
# 设置库名称