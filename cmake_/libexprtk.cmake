# 设置第三方库的路径
set(3RD_EXPRTK_DIR "${3RD_PATH}/exprtk" CACHE PATH "exprtk install path")
# 包含头文件目录
include_directories(${3RD_EXPRTK_DIR})
