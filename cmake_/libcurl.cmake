# 设置第三方库的路径
set(3RD_LIBCURL_DIR "${3RD_PATH}/curl-8.4.0" CACHE PATH "libcurl install path")
# 包含头文件目录
include_directories(${3RD_LIBCURL_DIR}/include)
# 链接库目录
link_directories("${3RD_LIBCURL_DIR}/lib/$<CONFIG>")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libcurl libcurl-d_imp)
######################################################
else ()
######################################################
set(libcurl libcurl_imp)
######################################################
endif ()