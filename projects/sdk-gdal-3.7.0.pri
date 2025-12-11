
win32: {

INCLUDEPATH += $$PWD/../../3rd/win64/gdal-3.7.0/include

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/gdal-3.7.0/lib/release -lgdal
}

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/gdal-3.7.0/lib/debug -lgdald
}
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/gdal-3.7.0/Debug/include/
        LIBS += -L$$PWD/../../3rd/lnx/gdal-3.7.0/Debug/lib -lgdald
        }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/gdal-3.7.0/Release/include/
        LIBS += -L$$PWD/../../3rd/lnx/gdal-3.7.0/Release/lib -lgdal
    }
}
