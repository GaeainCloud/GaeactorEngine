win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/gl/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/gl/lib/ -lOPENGL32
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/gl/lib/ -lOPENGL32
    }
}
