include(${CMAKE_DIR}/libarmadillo.cmake)
include(${CMAKE_DIR}/librapidjson.cmake)
# 设置第三方库的路径
set(BUILD_INCLUDE_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/include" CACHE PATH "build include install path")
set(BUILD_LIB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/lib" CACHE PATH "build lib install path")
set(BUILD_BIN_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/bin" CACHE PATH "build bin install path")
set(BUILD_PDB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/pdb" CACHE PATH "build pdb install path")
# 包含头文件目录
include_directories(${DynaModex_DIR}/gengine/phos-core)
include_directories(${DynaModex_DIR}/gengine/phos-core/plugin-core)
include_directories(${DynaModex_DIR}/gengine/phos-core/core)
include_directories(${DynaModex_DIR}/gengine/phos-core/core/frame)
include_directories(${DynaModex_DIR}/gengine/phos-core/core/frame/definitions)
include_directories(${DynaModex_DIR}/gengine/phos-core/core/frame/instances)
include_directories(${DynaModex_DIR}/gengine/phos-core/common)
include_directories(${DynaModex_DIR}/gengine/phos-core/jsonschema)
include_directories(${DynaModex_DIR}/gengine/phos-core/globals)
# 链接库目录
link_directories("${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(sdk_phos_core phos-cored)
######################################################
else ()
######################################################
set(sdk_phos_core phos-core)
######################################################
endif ()
