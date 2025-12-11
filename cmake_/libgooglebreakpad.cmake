# 设置第三方库的路径
set(3RD_GOOGLE_BREAKPAD_DIR "${3RD_PATH}/google_breakpad" CACHE PATH "Google Breakpad install path")
# 包含头文件目录
include_directories(${3RD_GOOGLE_BREAKPAD_DIR}/include)
# 链接库目录
link_directories("${3RD_GOOGLE_BREAKPAD_DIR}/lib/$<CONFIG>")
# 设置库名称
set(libgooglebreakpad common crash_generation_client exception_handler)