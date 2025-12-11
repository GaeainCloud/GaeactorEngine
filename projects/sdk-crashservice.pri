win32 {
    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/crashservice

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lcrashservice
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lcrashservice
    }
}

unix {
    INCLUDEPATH += \
        $$PWD/../../bin/lnx/sdk/src/crashservice
    
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/debug -lcrashservice
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/release -lcrashservice
    }
}
