win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/glfw-3.4/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/glfw-3.4/lib/debug -lglfw3dll
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/glfw-3.4/lib/release -lglfw3dll
    }
}
