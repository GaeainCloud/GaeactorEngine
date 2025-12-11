win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/QtXlsx/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/QtXlsx/lib/debug/ -lQt5Xlsxd
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/QtXlsx/lib/release/ -lQt5Xlsx
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/QtXlsx/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/QtXlsx/Debug/lib \
            -lQt5Xlsx
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/QtXlsx/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/QtXlsx/Release/lib \
            -lQt5Xlsx
    }
}

