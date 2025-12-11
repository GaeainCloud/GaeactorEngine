
win32: {
INCLUDEPATH += $$PWD/../../3rd/win64/osg_3.6.5_osgearth_2.10.2/include/osgearth_2.10.2

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/osg_3.6.5_osgearth_2.10.2/lib/osgearth_2.10.2/release -losgEarthAnnotation \
    -losgEarth \
    -losgEarthFeatures \
    -losgEarthSplat \
    -losgEarthSymbology \
    -losgEarthUtil 
}

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/osg_3.6.5_osgearth_2.10.2/lib/osgearth_2.10.2/debug -losgEarthAnnotationd \
    -losgEarthd \
    -losgEarthFeaturesd \
    -losgEarthSplatd \
    -losgEarthSymbologyd \
    -losgEarthUtild 
}
}

