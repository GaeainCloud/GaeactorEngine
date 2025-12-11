include(${CMAKE_DIR}/libboost.cmake)
# 设置第三方库的路径
set(3RD_WEBSOCKETPP_DIR "${3RD_PATH}/websocketpp-0.8.2" CACHE PATH "websocketpp install path")
# 包含头文件目录
include_directories(${3RD_WEBSOCKETPP_DIR}/include)
# 链接库目录

# 设置库名称