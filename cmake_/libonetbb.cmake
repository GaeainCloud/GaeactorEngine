# 设置第三方库的路径
set(3RD_ONETBB_DIR "${3RD_PATH}/oneTBB-2021.13.0" CACHE PATH "oneTBB install path")
# 包含头文件目录
include_directories(${3RD_ONETBB_DIR}/include)
# 链接库目录
link_directories("${3RD_ONETBB_DIR}/lib/$<CONFIG>")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libonetbb  tbb_debug tbb12_debug tbbmalloc_debug tbbmalloc_proxy_debug)
######################################################
else ()
######################################################
set(libonetbb tbb tbb12 tbbmalloc tbbmalloc_proxy)
######################################################
endif ()