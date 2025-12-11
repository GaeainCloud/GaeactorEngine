win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/zlib-1.3.0/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/zlib-1.3.0/lib/Debug/ -lzlibwapi
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/zlib-1.3.0/lib/Release/ -lzlibwapi
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/zlib-1.3.0/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/zlib-1.3.0/Debug/lib \
            -lz
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/zlib-1.3.0/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/zlib-1.3.0/Release/lib \
            -lz
    }
}
