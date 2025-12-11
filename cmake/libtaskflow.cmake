# 设置第三方库的路径
set(3RD_LIBTASKFLOW_DIR "${3RD_PATH}/taskflow-3.7.0" CACHE PATH "taskflow install path")
# 包含头文件目录
include_directories(${3RD_LIBTASKFLOW_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
