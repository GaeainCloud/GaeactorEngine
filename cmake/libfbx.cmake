# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_FBX_DIR "${3RD_PATH}/fbx" CACHE PATH "fbx install path")
    # 包含头文件目录
    include_directories(${3RD_FBX_DIR}/include)
    # 链接库目录
    # 设置库名称
else()
endif ()