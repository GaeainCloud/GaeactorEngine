set(3RD_LIBUV_DIR "${3RD_PATH}/libuv-1.46.0" CACHE PATH "libuv install path")
if (MSVC)
# 设置第三方库的路径
    # 包含头文件目录
    include_directories(${3RD_LIBUV_DIR}/include)
    # 链接库目录
    link_directories("${3RD_LIBUV_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
else()
    # 包含头文件目录
    include_directories(${3RD_LIBUV_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    # 链接库目录
    link_directories("${3RD_LIBUV_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    ######################################################
endif ()
# 设置库名称
set(libuv uv)