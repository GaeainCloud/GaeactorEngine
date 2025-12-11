
win32: {
INCLUDEPATH += $$PWD/../../3rd/win64/oneTBB-2021.13.0/include

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/oneTBB-2021.13.0/lib/debug -ltbb_debug -ltbb12_debug -ltbbmalloc_debug -ltbbmalloc_proxy_debug
}

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/oneTBB-2021.13.0/lib/release -ltbb -ltbb12 -ltbbmalloc -ltbbmalloc_proxy
}
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/oneTBB-2021.13/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/oneTBB-2021.13/Debug/lib -ltbb_debug -ltbbmalloc_debug -ltbbmalloc_proxy_debug
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/oneTBB-2021.13/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/oneTBB-2021.13/Release/lib -ltbb -ltbbmalloc -ltbbmalloc_proxy
    }
}
