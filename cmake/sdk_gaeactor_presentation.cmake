include(${CMAKE_DIR}/libosg.cmake)
include(${CMAKE_DIR}/libosgearth.cmake)
include(${CMAKE_DIR}/libgeos.cmake)
include(${CMAKE_DIR}/libgdal.cmake)
include(${CMAKE_DIR}/libassimp.cmake)
include(${CMAKE_DIR}/libfreeimage.cmake)
include(${CMAKE_DIR}/libfreetype2.cmake)
include(${CMAKE_DIR}/libstb.cmake)
include(${CMAKE_DIR}/libgl.cmake)
include(${CMAKE_DIR}/libglfw.cmake)
include(${CMAKE_DIR}/libglm.cmake)
include(${CMAKE_DIR}/libglad.cmake)
include(${CMAKE_DIR}/libkhr.cmake)
include(${CMAKE_DIR}/libfbx.cmake)
# 设置第三方库的路径
set(BUILD_INCLUDE_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/include" CACHE PATH "build include install path")
set(BUILD_LIB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/lib" CACHE PATH "build lib install path")
set(BUILD_BIN_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/bin" CACHE PATH "build bin install path")
set(BUILD_PDB_DIR "${DEPLOY_PATH}/${PLATFORM_DIR}/pdb" CACHE PATH "build pdb install path")
# 包含头文件目录
include_directories(${CppSharedArtifacts_DIR}/gaeactor-presentation)
# 链接库目录
link_directories("${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE_DIR}")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(sdk_gaeactor_presentation gaeactor-presentationd)
######################################################
else ()
######################################################
set(sdk_gaeactor_presentation gaeactor-presentation)
######################################################
endif ()