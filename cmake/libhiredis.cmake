# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_HIREDIS_DIR "${3RD_PATH}/hiredis" CACHE PATH "hiredis install path")
    # 包含头文件目录
    include_directories(${3RD_HIREDIS_DIR}/include)
    # 链接库目录
    link_directories("${3RD_HIREDIS_DIR}/lib")
    # 设置库名称
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ######################################################
    set(libhiredis hiredisd ws2_32)
    ######################################################
    else ()
    ######################################################
    set(libhiredis hiredis ws2_32)
    ######################################################
    endif ()
else()
    # 设置第三方库的路径
    set(3RD_HIREDIS_DIR "${3RD_PATH}/hiredis" CACHE PATH "hiredis install path")
    # 包含头文件目录
    include_directories(${3RD_HIREDIS_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    # 链接库目录
    link_directories("${3RD_HIREDIS_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    # 设置库名称
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ######################################################
    set(libhiredis hiredisd)
    ######################################################
    else ()
    ######################################################
    set(libhiredis hiredis)
    ######################################################
    endif ()
endif ()