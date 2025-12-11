# 设置第三方库的路径
set(3RD_GLFW_DIR "${3RD_PATH}/glfw-3.4" CACHE PATH "glfw install path")
# 包含头文件目录
include_directories(${3RD_GLFW_DIR}/include)
# 链接库目录
link_directories("${3RD_GLFW_DIR}/lib/$<CONFIG>")
# 设置库名称
set(libglfw glfw3dll)