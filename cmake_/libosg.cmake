# 设置第三方库的路径
set(3RD_OSG_DIR "${3RD_PATH}/osg_3.6.5_osgearth_2.10.2" CACHE PATH "osg install path")
# 包含头文件目录
include_directories(${3RD_OSG_DIR}/include/osg_3.6.5)
# 链接库目录
link_directories("${3RD_OSG_DIR}/lib/osg_3.6.5/$<CONFIG>")
# 设置库名称
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
######################################################
set(libosg
        OpenThreadsd
        osgAnimationd
        osgFXd
        osgManipulatord
        osgPresentationd
        osgSimd
        osgTextd
        osgUtild
        osgVolumed
        osgd
        osgDBd
        osgGAd
        osgParticled
        osgShadowd
        osgTerraind
        osgUId
        osgViewerd
        osgWidgetd)
######################################################
else ()
######################################################
set(libosg
        OpenThreads
        osgAnimation
        osgFX
        osgManipulator
        osgPresentation
        osgSim
        osgText
        osgUtil
        osgVolume
        osg
        osgDB
        osgGA
        osgParticle
        osgShadow
        osgTerrain
        osgUI
        osgViewer
        osgWidget)
######################################################
endif ()