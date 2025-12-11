
win32: {

INCLUDEPATH += $$PWD/../../3rd/win64/geos-3.11.2/include

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/geos-3.11.2/lib/release -lgeos
}

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/geos-3.11.2/lib/debug -lgeos
}
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/geos-3.11.2/Debug/include/
        LIBS += -L$$PWD/../../3rd/lnx/geos-3.11.2/Debug/lib -lgeos_c \
        -lgeos
        }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/geos-3.11.2/Release/include/
        LIBS += -L$$PWD/../../3rd/lnx/geos-3.11.2/Release/lib -lgeos_c \
        -lgeos
    }
}
