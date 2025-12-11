# 设置第三方库的路径
set(BUILD_INCLUDE_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/include" CACHE PATH "build include install path")
set(BUILD_LIB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/lib" CACHE PATH "build lib install path")
set(BUILD_BIN_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/bin" CACHE PATH "build bin install path")
set(BUILD_PDB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/pdb" CACHE PATH "build pdb install path")
# 包含头文件目录
include_directories(${gaeactor_DIR}/gaeactor-environment)
# 链接库目录
link_directories("${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(sdk_gaeactor_environment gaeactor-environmentd)
######################################################
else ()
######################################################
set(sdk_gaeactor_environment gaeactor-environment)
######################################################
endif ()