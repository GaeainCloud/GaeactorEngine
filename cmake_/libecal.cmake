include(${CMAKE_DIR}/libboost.cmake)
include(${CMAKE_DIR}/libmsgpack.cmake)

if (MSVC)
    add_compile_definitions (PROTOBUF_USE_DLLS)
endif ()

# 设置第三方库的路径
set(3RD_ECAL_DIR "${3RD_PATH}/ecal-5.12.4" CACHE PATH "ecal-5.12.4 install path")
# 包含头文件目录
include_directories(${3RD_ECAL_DIR}/include)
# 链接库目录
link_directories("${3RD_ECAL_DIR}/lib/$<CONFIG>")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libecal 
    ecal_core_cd 
    ecal_cored 
    ecal_ecaltime_pbd 
    ecal_mon_plugin_libd 
    ecal_protod 
    hdf5_D 
    hdf5_cpp_D 
    libprotobuf-lited 
    libprotocd 
    ecal_app_pbd 
    ecal_core_pbd 
    ecal_ecal-utilsd 
    ecal_hdf5d 
    ecal_pbd 
    ecal_rec_addon_cored 
    libprotobufd)
######################################################
else ()
######################################################
set(libecal 
    ecal_core_c
    ecal_core
    ecal_ecaltime_pb
    ecal_mon_plugin_lib
    ecal_proto
    hdf5
    hdf5_cpp
    libprotobuf-lite
    libprotoc
    ecal_app_pb
    ecal_core_pb
    ecal_ecal-utils
    ecal_hdf5
    ecal_pb
    ecal_rec_addon_core
    libprotobuf)
######################################################
endif ()