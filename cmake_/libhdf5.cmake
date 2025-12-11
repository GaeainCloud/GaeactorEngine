if (MSVC)
    # 设置第三方库的路径
    set(3RD_HDF5_DIR "${3RD_PATH}/HDF5-1.13.0" CACHE PATH "hdf5 install path")
    # 包含头文件目录
    include_directories(${3RD_HDF5_DIR}/include)
    # 链接库目录
    link_directories("${3RD_HDF5_DIR}/lib/$<CONFIG>")
    # 设置库名称
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ######################################################
    set(libhdf5 hdf5_cpp_D hdf5_hl_D hdf5_tools_D  hdf5_hl_cpp_D)
    ######################################################
    else ()
    ######################################################
    set(libhdf5 hdf5 hdf5_cpp hdf5_hl hdf5_tools hdf5_hl_cpp)
    ######################################################
    endif ()
else()

endif ()