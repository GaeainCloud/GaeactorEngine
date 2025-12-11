if (MSVC)
    # 设置第三方库的路径
    set(3RD_NETCDFC_DIR "${3RD_PATH}/netcdf-c-4.9.2" CACHE PATH "netcdf-c install path")
    # 包含头文件目录
    include_directories(${3RD_NETCDFC_DIR}/include)
    # 链接库目录
    link_directories("${3RD_NETCDFC_DIR}/lib/$<CONFIG>")
    set(libnetcdfc netcdf)
else()

endif ()