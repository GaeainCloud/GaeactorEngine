win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/libuv-1.46.0/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/libuv-1.46.0/lib/Debug \
            -luv 
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/libuv-1.46.0/lib/Release \
            -luv 
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/libuv-1.46.0/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/libuv-1.46.0/Debug/lib \
            -luv 
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/libuv-1.46.0/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/libuv-1.46.0/Release/lib \
            -luv 

    }
}
