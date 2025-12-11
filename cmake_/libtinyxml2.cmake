if (MSVC)

else()
# 设置第三方库的路径
set(3RD_TINYXML2_DIR "${3RD_PATH}/tinyxml2-9.0.0/$<CONFIG>" CACHE PATH "tinyxml2 install path")
# 包含头文件目录
include_directories(${3RD_TINYXML2_DIR}/include)
# 链接库目录
link_directories("${3RD_TINYXML2_DIR}/lib")
# 设置库名称
######################################################
set(libtinyxml2 tinyxml2)
######################################################
endif ()