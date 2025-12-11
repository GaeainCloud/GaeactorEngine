win32 {

    DEFINES += USING_GAEACTOR_EXPORT_LIB

    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/gaeactor

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lgaeactor
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lgaeactor
    }
}

unix {
    DEFINES += USING_GAEACTOR_EXPORT_LIB

    INCLUDEPATH += \
        $$PWD/../../bin/lnx/sdk/src/gaeactor
    
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/debug -lgaeactor
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/release -lgaeactor
    }
}
