# 设置第三方库的路径
set(3RD_BOOST_DIR "${3RD_PATH}/boost-1.84.0" CACHE PATH "boost install path")
# 包含头文件目录
include_directories(${3RD_BOOST_DIR}/include/boost-1_84)
# 链接库目录

# 设置库名称