# 设置第三方库的路径
set(3RD_FREEIMAGE_DIR "${3RD_PATH}/FreeImage" CACHE PATH "FreeImage install path")
# 包含头文件目录
include_directories(${3RD_FREEIMAGE_DIR}/include)
# 链接库目录
link_directories("${3RD_FREEIMAGE_DIR}/lib/x64")
# 设置库名称
set(libfreeimage FreeImage)