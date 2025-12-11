
win32: {

    INCLUDEPATH += $$PWD/../../3rd/win64/concurrentqueue-master

}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/concurrentqueue-master
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/concurrentqueue-master
    }
}
