# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_GOOGLE_BREAKPAD_DIR "${3RD_PATH}/google_breakpad" CACHE PATH "Google Breakpad install path")
    # 包含头文件目录
    include_directories(${3RD_GOOGLE_BREAKPAD_DIR}/include)
    # 链接库目录
    link_directories("${3RD_GOOGLE_BREAKPAD_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
    # 设置库名称
    set(libgooglebreakpad common crash_generation_client exception_handler)
else()
    # 设置第三方库的路径
    set(3RD_GOOGLE_BREAKPAD_DIR "${3RD_PATH}/breakpad-2023.06.01" CACHE PATH "Google Breakpad install path")
    string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)
    # 包含头文件目录
    include_directories(${3RD_GOOGLE_BREAKPAD_DIR}/${BUILD_TYPE_LOWER}/include/breakpad)
    # 链接库目录
    link_directories("${3RD_GOOGLE_BREAKPAD_DIR}/${BUILD_TYPE_LOWER}/lib")
    # 设置库名称
    set(libgooglebreakpad breakpad breakpad_client)
endif ()