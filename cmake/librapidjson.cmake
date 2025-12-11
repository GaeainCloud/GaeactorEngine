if (MSVC)
    # 设置第三方库的路径
    set(3RD_LIBRAPIDJSON_DIR "${3RD_PATH}/rapidjson-1.1.0" CACHE PATH "rapidjson-1.1.0 install path")
    # 包含头文件目录
    include_directories(${3RD_LIBRAPIDJSON_DIR}/include)
else()
    # 设置第三方库的路径
    set(3RD_LIBRAPIDJSON_DIR "${3RD_PATH}/rapidjson" CACHE PATH "rapidjson-1.1.0 install path")
    # 包含头文件目录
    include_directories(${3RD_LIBRAPIDJSON_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
endif ()