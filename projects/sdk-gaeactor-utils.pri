
win32 {

    INCLUDEPATH += $$PWD/../../3rd/win64/hiredis/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/hiredis/lib \
            -lhiredisd \
            -lws2_32 \
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/hiredis/lib \
            -lhiredis \
            -lws2_32 \
    }

    INCLUDEPATH += \
        $$PWD/../../bin/win64/sdk/src/gaeactor-utils \
        $$PWD/../../bin/win64/sdk/src/gaeactor-utils/src

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/debug -lgaeactor-utils
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/win64/sdk/lib/release -lgaeactor-utils
    }
}

unix {
    INCLUDEPATH += \
        $$PWD/../../bin/lnx/sdk/src/gaeactor-utils \
        $$PWD/../../bin/lnx/sdk/src/gaeactor-utils/src

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/debug -lgaeactor-utils
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../bin/lnx/sdk/lib/release -lgaeactor-utils
    }
}


