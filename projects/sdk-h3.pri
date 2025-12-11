
win32: {
INCLUDEPATH += $$PWD/../../3rd/win64/h3-4.1.0/include/h3

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/h3-4.1.0/lib/static_release -lh3
}

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/h3-4.1.0/lib/static_debug -lh3
}
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/h3-4.1.0/Debug/include/h3
        LIBS += -L$$PWD/../../3rd/lnx/h3-4.1.0/Debug/lib -lh3
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/h3-4.1.0/Release/include/h3
        LIBS += -L$$PWD/../../3rd/lnx/h3-4.1.0/Release/lib -lh3
    }
}
