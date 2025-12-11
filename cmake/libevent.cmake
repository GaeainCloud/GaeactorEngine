# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_LIBEVENT_DIR "${3RD_PATH}/libevent" CACHE PATH "libevent install path")
    # 包含头文件目录
    include_directories(${3RD_LIBEVENT_DIR}/include)
    # 链接库目录
    link_directories("${3RD_LIBEVENT_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    set(libevent event event_core event_extra event_openssl)
else()
    # 设置第三方库的路径
    set(3RD_LIBEVENT_DIR "${3RD_PATH}/libevent-2.1.12" CACHE PATH "libevent install path")
    # 包含头文件目录
    include_directories(${3RD_LIBEVENT_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    # 链接库目录
    link_directories("${3RD_LIBEVENT_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    # 设置库名称
    set(libevent event event_core event_extra event_openssl)
endif ()