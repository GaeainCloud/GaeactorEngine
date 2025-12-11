if (MSVC)
  add_compile_definitions (NOMINMAX)
endif ()
# 设置第三方库的路径
set(3RD_CXXGRAPH_DIR "${3RD_PATH}/CXXGraph-3.0.0" CACHE PATH "CXXGraph-3.0.0 install path")
# 包含头文件目录
include_directories(${3RD_CXXGRAPH_DIR}/include)
