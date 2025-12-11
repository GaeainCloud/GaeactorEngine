# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_FREETYPE2_DIR "${3RD_PATH}/freetype2" CACHE PATH "freetype2 install path")
    # 包含头文件目录
    include_directories(${3RD_FREETYPE2_DIR}/include)
    # 链接库目录
    link_directories("${3RD_FREETYPE2_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ######################################################
    set(libfreetype2 freetyped)
    ######################################################
    else ()
    ######################################################
    set(libfreetype2 freetype)
    ######################################################
    endif ()
else()
endif ()