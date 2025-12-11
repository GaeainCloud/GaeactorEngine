win32 {
    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/SpaceIndexerArcadix

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lSpaceIndexerArcadixLib
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lSpaceIndexerArcadixLib
    }
}



