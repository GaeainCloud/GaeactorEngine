win32: {
DEFINES += NOMINMAX
    INCLUDEPATH += $$PWD/../../3rd/win64/CXXGraph-3.0.0/include
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/CXXGraph-3.0.0/Debug/include
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/CXXGraph-3.0.0/Release/include
    }
}
