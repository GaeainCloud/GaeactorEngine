win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/kdchart-2.8.0/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/kdchart-2.8.0/lib/Debug/ -lkdchart
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/kdchart-2.8.0/lib/Release/ -lkdchart2
    }
}


