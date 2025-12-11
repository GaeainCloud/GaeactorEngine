include($$PWD/sdk-ecal_5.12.4.pri)

win32 {
    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/gaeactor-comm

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lgaeactor-comm
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lgaeactor-comm
    }
}

unix {
    INCLUDEPATH += \
        $$PWD/../../bin/lnx/sdk/src/gaeactor-comm
    
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/debug -lgaeactor-comm
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/release -lgaeactor-comm
    }
}
