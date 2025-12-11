if (MSVC)
    # 设置第三方库的路径
    # set(3RD_SPLINE_DIR "${3RD_PATH}/spline-master" CACHE PATH "spline install path")
    set(3RD_CXX_SPLINE_DIR "${3RD_PATH}/cxx-spline-master" CACHE PATH "cxx-spline install path")
    # 包含头文件目录
    # include_directories(${3RD_SPLINE_DIR}/src)
    include_directories(${3RD_CXX_SPLINE_DIR}/include)
else()
    # 设置第三方库的路径
    set(3RD_CXX_SPLINE_DIR "${3RD_PATH}/cxx-spline-master" CACHE PATH "cxx-spline install path")
    # 包含头文件目录
    include_directories(${3RD_CXX_SPLINE_DIR}/include)
endif ()