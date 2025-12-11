
if (MSVC)
add_compile_definitions(WIN32_LEAN_AND_MEAN)  
endif ()
# 设置第三方库的路径
set(3RD_LIBHV_DIR "${3RD_PATH}/libhv-1.3.0" CACHE PATH "libhv install path")
# 包含头文件目录
include_directories(${3RD_LIBHV_DIR}/build_bin/include)
include_directories(${3RD_LIBHV_DIR}/build_bin/include/evpp)
include_directories(${3RD_LIBHV_DIR}/build_bin/include/hv)
# 链接库目录
link_directories("${3RD_LIBHV_DIR}/build_bin/lib/$<CONFIG>")
# 设置库名称
set(libhv hv)