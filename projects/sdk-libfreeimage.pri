win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/FreeImage/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/FreeImage/lib/x64/ -lFreeImage
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/FreeImage/lib/x64/ -lFreeImage
    }
}
