# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_EASY_PROFILER_DIR "${3RD_PATH}/easy_profiler-v2.1.0-msvc22-win64" CACHE PATH "easyprofiler install path")
    # 包含头文件目录
    include_directories(${3RD_EASY_PROFILER_DIR}/include)
    # 链接库目录
    link_directories("${3RD_EASY_PROFILER_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    set(libeasyprofiler easy_profiler)
else()
    # 设置第三方库的路径
    set(3RD_EASY_PROFILER_DIR "${3RD_PATH}/easy_profiler-2.1.0" CACHE PATH "easyprofiler install path")
    # 包含头文件目录
    include_directories(${3RD_EASY_PROFILER_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    # 链接库目录
    link_directories("${3RD_EASY_PROFILER_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    # 设置库名称
    set(libeasyprofiler easy_profiler)
endif ()