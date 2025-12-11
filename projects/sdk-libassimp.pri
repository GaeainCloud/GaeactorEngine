win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/assimp-5.4.3/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/assimp-5.4.3/lib/debug -lassimp-vc143-mtd -lzlibstaticd
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/assimp-5.4.3/lib/release/ -lassimp-vc143-mt -lzlibstatic
    }
}
