# 设置第三方库的路径
set(3RD_GL_DIR "${3RD_PATH}/gl" CACHE PATH "gl install path")
# 包含头文件目录
include_directories(${3RD_GL_DIR}/include)
# 链接库目录
link_directories("${3RD_GL_DIR}/lib")
# 设置库名称
set(libgl OPENGL32)