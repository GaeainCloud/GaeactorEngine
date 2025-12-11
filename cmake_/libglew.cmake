# 设置第三方库的路径
set(3RD_GLEW_DIR "${3RD_PATH}/glew-2.1.0" CACHE PATH "glew install path")
# 包含头文件目录
include_directories(${3RD_GLEW_DIR}/include)
# 链接库目录
link_directories("${3RD_GLEW_DIR}/lib/$<CONFIG>/x64")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libglew glew32d glew32sd)
######################################################
else ()
######################################################
set(libglew glew32 glew32s)
######################################################
endif ()