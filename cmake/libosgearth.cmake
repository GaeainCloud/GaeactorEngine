if (MSVC)
        # 设置第三方库的路径
        set(3RD_OSGEARTH_DIR "${3RD_PATH}/osg_3.6.5_osgearth_2.10.2" CACHE PATH "osgearth install path")
        # 包含头文件目录
        include_directories(${3RD_OSGEARTH_DIR}/include/osgearth_2.10.2)
        # 链接库目录
        link_directories("${3RD_OSGEARTH_DIR}/lib/osgearth_2.10.2/${CMAKE_BUILD_TYPE_DIR}")
        # 设置库名称
        if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        ######################################################
        set(libosgearth
                osgEarthAnnotationd
                osgEarthd
                osgEarthFeaturesd
                osgEarthSplatd
                osgEarthSymbologyd
                osgEarthUtild)
        ######################################################
        else ()
        ######################################################
        set(libosgearth
                osgEarthAnnotation
                osgEarth
                osgEarthFeatures
                osgEarthSplat
                osgEarthSymbology
                osgEarthUtil)
        ######################################################
        endif ()
else()

endif ()