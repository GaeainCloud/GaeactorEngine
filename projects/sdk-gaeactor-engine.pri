win32 {
    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/gaeactor-engine

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lgaeactor-engine
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lgaeactor-engine
    }
}

unix {
    INCLUDEPATH += \
        $$PWD/../../bin/lnx/sdk/src/gaeactor-engine
    
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/debug -lgaeactor-engine
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/release -lgaeactor-engine
    }
}
