include($$PWD/sdk-osg_3.6.5.pri)
include($$PWD/sdk-osgearth_2.10.2.pri)
include($$PWD/sdk-geos-3.11.2.pri)
include($$PWD/sdk-gdal-3.7.0.pri)

include($$PWD/sdk-libassimp.pri)
include($$PWD/sdk-libfreeimage.pri)
include($$PWD/sdk-libfreetype2.pri)
include($$PWD/sdk-libstb.pri)
include($$PWD/sdk-libgl.pri)
include($$PWD/sdk-libglfw.pri)
include($$PWD/sdk-libglm.pri)
include($$PWD/sdk-libglad.pri)
include($$PWD/sdk-libkhr.pri)
include($$PWD/sdk-libfbx.pri)
win32 {
    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/gaeactor-presentation

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lgaeactor-presentation
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lgaeactor-presentation
    }
}

unix {
    INCLUDEPATH += \
        $$PWD/../../bin/lnx/sdk/src/gaeactor-presentation

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/debug -lgaeactor-presentation
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/release -lgaeactor-presentation
    }
}


