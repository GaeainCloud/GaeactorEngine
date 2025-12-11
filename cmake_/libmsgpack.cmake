# 设置第三方库的路径
set(3RD_MSGPACK_DIR "${3RD_PATH}/msgpack-c-cpp-6.1.1" CACHE PATH "msgpack-c-cpp-6.1.1 install path")
# 包含头文件目录
include_directories(${3RD_MSGPACK_DIR}/include)