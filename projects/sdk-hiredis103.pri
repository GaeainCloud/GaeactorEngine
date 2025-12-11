win32: {
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
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/hiredis/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/hiredis/Debug/lib -lhiredisd
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/hiredis/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/hiredis/Release/lib -lhiredis
    }
}
