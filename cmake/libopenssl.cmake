if (MSVC)
    # 设置第三方库的路径
    set(3RD_LIBOPENSSL_DIR "${3RD_PATH}/openssl1.1.1u" CACHE PATH "libopenssl install path")
    # 包含头文件目录
    include_directories(${3RD_LIBOPENSSL_DIR}/include)
    # 链接库目录
    link_directories("${3RD_LIBOPENSSL_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    set(libopenssl libcrypto libssl)
else()
    # 设置第三方库的路径
    set(3RD_LIBOPENSSL_DIR "${3RD_PATH}/openssl1.1.1u" CACHE PATH "libopenssl install path")    
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)
    # 包含头文件目录
    include_directories(${3RD_LIBOPENSSL_DIR}/${BUILD_TYPE_LOWER}/include)
    # 链接库目录
    link_directories("${3RD_LIBOPENSSL_DIR}/${BUILD_TYPE_LOWER}/lib")
    # 设置库名称
    set(libopenssl crypto ssl)
endif ()