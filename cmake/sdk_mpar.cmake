# 设置第三方库的路径
set(BUILD_INCLUDE_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/include" CACHE PATH "build include install path")
set(BUILD_LIB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/lib" CACHE PATH "build lib install path")
set(BUILD_BIN_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/bin" CACHE PATH "build bin install path")
set(BUILD_PDB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/pdb" CACHE PATH "build pdb install path")
# 包含头文件目录
include_directories(${Mapr_DIR}/mparcore)
include_directories(${Mapr_DIR}/mparcore/RuntimeObjects)
include_directories(${Mapr_DIR}/mparcore/ParameterSettings)
include_directories(${Mapr_DIR}/mparcore/Utilities)
# 链接库目录
link_directories("${CMAKE_BINARY_DIR}/lib/$<CONFIG>")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(sdk_mpar mparcored)
######################################################
else ()
######################################################
set(sdk_mpar mparcore)
######################################################
endif ()