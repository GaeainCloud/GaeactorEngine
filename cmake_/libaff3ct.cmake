if (MSVC)
    # 设置第三方库的路径
    set(3RD_AFF3CT_DIR "${3RD_PATH}/aff3ct" CACHE PATH "aff3ct install path")
    # 包含头文件目录
    include_directories(${3RD_AFF3CT_DIR}/$<CONFIG>/include/aff3ct-3.0.2/aff3ct)
    include_directories(${3RD_AFF3CT_DIR}/$<CONFIG>/include/aff3ct-3.0.2/cli)
    include_directories(${3RD_AFF3CT_DIR}/$<CONFIG>/include/aff3ct-3.0.2/MIPP)
    include_directories(${3RD_AFF3CT_DIR}/$<CONFIG>/include/aff3ct-3.0.2/date)
    include_directories(${3RD_AFF3CT_DIR}/$<CONFIG>/include/aff3ct-3.0.2/rang)
    # 链接库目录
    link_directories("${3RD_AFF3CT_DIR}/$<CONFIG>/lib")
    set(libaff3ct aff3ct-3.0.2)
else()

endif ()