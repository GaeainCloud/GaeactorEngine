if (MSVC)
    # 设置第三方库的路径
    set(3RD_PYBIND11_DIR "${3RD_PATH}/pybind11-master" CACHE PATH "pybind11 install path")
    set(3RD_PYTHON_DIR "${3RD_PATH}/python_3_12_2" CACHE PATH "python install path")
    # 包含头文件目录
    include_directories(${3RD_PYBIND11_DIR}/include)
    include_directories(${3RD_PYTHON_DIR}/include)
    # 链接库目录
    link_directories("${3RD_PYTHON_DIR}/libs")
    set(libpybind11 python312)
else()

endif ()