win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/glew-2.1.0/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/glew-2.1.0/lib/Debug/x64 -lglew32d -lglew32sd
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/glew-2.1.0/lib/Release/x64 -lglew32 -lglew32s
    }
}
