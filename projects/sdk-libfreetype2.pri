win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/freetype2/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/freetype2/lib/debug/ -lfreetyped
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/freetype2/lib/release/ -lfreetype
    }
}
