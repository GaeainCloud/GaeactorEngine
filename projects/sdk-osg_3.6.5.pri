
win32: {
INCLUDEPATH += $$PWD/../../3rd/win64/osg_3.6.5_osgearth_2.10.2/include/osg_3.6.5

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/osg_3.6.5_osgearth_2.10.2/lib/osg_3.6.5/release -lOpenThreads \
    -losgAnimation \
    -losgFX \
    -losgManipulator \
    -losgPresentation \
    -losgSim \
    -losgText \
    -losgUtil \
    -losgVolume \
    -losg \
    -losgDB \
    -losgGA \
    -losgParticle \
    -losgShadow \
    -losgTerrain \
    -losgUI \
    -losgViewer \
    -losgWidget
}

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/osg_3.6.5_osgearth_2.10.2/lib/osg_3.6.5/debug -lOpenThreadsd \
    -losgAnimationd \
    -losgFXd \
    -losgManipulatord \
    -losgPresentationd \
    -losgSimd \
    -losgTextd \
    -losgUtild \
    -losgVolumed \
    -losgd \
    -losgDBd \
    -losgGAd \
    -losgParticled \
    -losgShadowd \
    -losgTerraind \
    -losgUId \
    -losgViewerd \
    -losgWidgetd
}
}

