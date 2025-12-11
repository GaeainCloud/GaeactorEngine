if (MSVC)
# 设置第三方库的路径
set(3RD_FASTRTPS_DIR "${3RD_PATH}/fastrtps_2.10.1" CACHE PATH "fastrtps install path")
# 包含头文件目录
include_directories(${3RD_FASTRTPS_DIR}/include)
# 链接库目录
link_directories("${3RD_FASTRTPS_DIR}/lib/x64Win64VS2019")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libfastrtps fastrtpsd-2.10 foonathan_memory-0.7.3-dbg Advapi32)
######################################################
else ()
######################################################
set(libfastrtps fastrtps-2.10 foonathan_memory-0.7.3 Advapi32)
######################################################
endif ()
else()

endif ()