# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_ASSIMP_DIR "${3RD_PATH}/assimp-5.4.3" CACHE PATH "assimp install path")
    # 包含头文件目录
    include_directories(${3RD_ASSIMP_DIR}/include)
    # 链接库目录
    link_directories("${3RD_ASSIMP_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ######################################################
    set(libassimp assimp-vc143-mtd zlibstaticd)
    ######################################################
    else ()
    ######################################################
    set(libassimp assimp-vc143-mt zlibstatic)
    ######################################################
    endif ()
else()

endif ()