if (MSVC)
    # 设置第三方库的路径
    set(3RD_OATPP_DIR "${3RD_PATH}/oatpp-1.3.0" CACHE PATH "oatpp-1.3.0 install path")
    set(3RD_OATPP_SWAGGER_DIR "${3RD_PATH}/oatpp-swagger-1.3.0" CACHE PATH "oatpp-swagger-1.3.0 install path")
    set(3RD_OATPP_SQLITE_DIR "${3RD_PATH}/oatpp-sqlite-1.3.0" CACHE PATH "oatpp-sqlite-1.3.0 install path")
    set(3RD_OATPP_WEBSOCKET_DIR "${3RD_PATH}/oatpp-websocket-1.3.0" CACHE PATH "oatpp-websocket-1.3.0 install path")
    # 包含头文件目录
    include_directories(${3RD_OATPP_DIR}/include/oatpp-1.3.0/oatpp)
    include_directories(${3RD_OATPP_SWAGGER_DIR}/include/oatpp-1.3.0/oatpp-swagger)
    include_directories(${3RD_OATPP_SQLITE_DIR}/include/oatpp-1.3.0/oatpp-sqlite)
    include_directories(${3RD_OATPP_SQLITE_DIR}/include/oatpp-1.3.0/oatpp-sqlite/sqlite)
    include_directories(${3RD_OATPP_WEBSOCKET_DIR}/include/include/oatpp-1.3.0/oatpp-websocket)
    # 链接库目录
    link_directories("${3RD_OATPP_DIR}/lib/oatpp-1.3.0/${CMAKE_BUILD_TYPE_DIR}")
    link_directories("${3RD_OATPP_SWAGGER_DIR}/lib/oatpp-1.3.0/${CMAKE_BUILD_TYPE_DIR}")
    link_directories("${3RD_OATPP_SQLITE_DIR}/lib/oatpp-1.3.0/${CMAKE_BUILD_TYPE_DIR}")
    link_directories("${3RD_OATPP_WEBSOCKET_DIR}/lib/oatpp-1.3.0/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    ######################################################
    set(liboatpp oatpp oatpp-swagger oatpp-sqlite sqlite oatpp-websocket)
    ######################################################
else()
    # 设置第三方库的路径
    set(3RD_OATPP_DIR "${3RD_PATH}/oatpp" CACHE PATH "oatpp-1.3.0 install path")
    set(3RD_OATPP_SWAGGER_DIR "${3RD_PATH}/oatpp-swagger" CACHE PATH "oatpp-swagger-1.3.0 install path")
    # 包含头文件目录
    include_directories(${3RD_OATPP_DIR}/${CMAKE_BUILD_TYPE_DIR}/include/oatpp-1.3.0/oatpp)
    include_directories(${3RD_OATPP_SWAGGER_DIR}/${CMAKE_BUILD_TYPE_DIR}/include/oatpp-1.3.0/oatpp-swagger)
    # 链接库目录
    link_directories("${3RD_OATPP_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib/oatpp-1.3.0")
    link_directories("${3RD_OATPP_SWAGGER_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib/oatpp-1.3.0")
    # 设置库名称
    ######################################################
    set(liboatpp oatpp oatpp-swagger)
    ######################################################
endif ()