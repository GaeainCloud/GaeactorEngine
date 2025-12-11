win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/libuSockets-0.8.6/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/libuSockets-0.8.6/lib/Debug/libuSocket.lib
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/libuSockets-0.8.6/lib/Release/libuSocket.lib
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../3rd/lnx/libuSockets-0.8.6/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/libuSockets-0.8.6/Debug/lib \
            -luSocket 
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../3rd/lnx/libuSockets-0.8.6/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/libuSockets-0.8.6/Release/lib \
            -luSocket 

    }
}