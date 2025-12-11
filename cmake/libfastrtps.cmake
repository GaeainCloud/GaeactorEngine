# 设置第三方库的路径
if (MSVC)
    # 设置第三方库的路径
    set(3RD_FASTRTPS_DIR "${3RD_PATH}/fastrtps_2.10.1" CACHE PATH "fastrtps install path")
    # 包含头文件目录
    include_directories(${3RD_FASTRTPS_DIR}/include)
    # 链接库目录
    link_directories("${3RD_FASTRTPS_DIR}/lib/x64Win64VS2019")
    # 设置库名称
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ######################################################
    set(libfastrtps fastrtpsd-2.10 foonathan_memory-0.7.3-dbg Advapi32)
    ######################################################
    else ()
    ######################################################
    set(libfastrtps fastrtps-2.10 foonathan_memory-0.7.3 Advapi32)
    ######################################################
    endif ()
else()
    # 设置第三方库的路径
    set(3RD_FAST_CDR_DIR "${3RD_PATH}/Fast-CDR-1.1.1" CACHE PATH "Fast-CDR install path")
    set(3RD_MEMORY_DIR "${3RD_PATH}/memory-0.7-3" CACHE PATH "memory install path")
    set(3RD_FAST_DDS_DIR "${3RD_PATH}/Fast-DDS-2.10.2" CACHE PATH "Fast-DDS install path")
    # 包含头文件目录
    include_directories(${3RD_FAST_CDR_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    include_directories(${3RD_MEMORY_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    include_directories(${3RD_FAST_DDS_DIR}/${CMAKE_BUILD_TYPE_DIR}/include)
    # 链接库目录
    link_directories("${3RD_FAST_CDR_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    link_directories("${3RD_MEMORY_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    link_directories("${3RD_FAST_DDS_DIR}/${CMAKE_BUILD_TYPE_DIR}/lib")
    # 设置库名称
    # 设置库名称
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ######################################################
    set(libfastrtps fastcdr foonathan_memory-0.7.3-dbg fastrtps)
    ######################################################
    else ()
    ######################################################
    set(libfastrtps fastcdr foonathan_memory-0.7.3 fastrtps)
    ######################################################
    endif ()
endif ()