# 设置第三方库的路径
set(BUILD_INCLUDE_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/include" CACHE PATH "build include install path")
set(BUILD_LIB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/lib" CACHE PATH "build lib install path")
set(BUILD_BIN_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/bin" CACHE PATH "build bin install path")
set(BUILD_PDB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/pdb" CACHE PATH "build pdb install path")
# 包含头文件目录
include_directories(${CppSharedArtifacts_DIR}/gaeactor-utils)
include_directories(${CppSharedArtifacts_DIR}/gaeactor-utils/src)
