win32 {
    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/gaeactor-agent-sensors

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lgaeactor-agent-sensors
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lgaeactor-agent-sensors
    }
}

unix {
    INCLUDEPATH += \
        $$PWD/../../bin/lnx/sdk/src/gaeactor-agent-sensors
    
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/debug -lgaeactor-agent-sensors
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/release -lgaeactor-agent-sensors
    }
}
