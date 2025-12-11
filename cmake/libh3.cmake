# 设置第三方库的路径
set(3RD_H3_DIR "${3RD_PATH}/h3-4.1.0" CACHE PATH "h3 install path")
# 设置第三方库的路径
if (MSVC)
    # 包含头文件目录
    include_directories(${3RD_H3_DIR}/include/h3)
    # 链接库目录
    link_directories("${3RD_H3_DIR}/lib/static_${CMAKE_BUILD_TYPE}")
else()
    # 包含头文件目录
    include_directories(${3RD_H3_DIR}/${CMAKE_BUILD_TYPE_DIR}/include/h3)
    # 链接库目录
    link_directories("${3RD_H3_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
endif ()
# 设置库名称
set(libh3 h3)
