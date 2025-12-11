include(${CMAKE_DIR}/libclapack.cmake)
# 设置第三方库的路径
set(3RD_ARMADILLO_DIR "${3RD_PATH}/armadillo-12.6.2" CACHE PATH "armadillo install path")
# 包含头文件目录
include_directories(${3RD_ARMADILLO_DIR}/include)
