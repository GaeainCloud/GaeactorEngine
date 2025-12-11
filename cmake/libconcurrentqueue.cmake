# 设置第三方库的路径
set(3RD_CONCURRENTQUEUE_DIR "${3RD_PATH}/concurrentqueue-master" CACHE PATH "concurrentqueue install path")
# 包含头文件目录
include_directories(${3RD_CONCURRENTQUEUE_DIR})
