
win32: {

    INCLUDEPATH += $$PWD/../../3rd/win64/easy_profiler-v2.1.0-msvc22-win64/include
CONFIG(release, debug|release):{
    LIBS += -L$$PWD/../../3rd/win64/easy_profiler-v2.1.0-msvc22-win64/lib/release/ -leasy_profiler
}
CONFIG(debug, debug|release):{
    LIBS += -L$$PWD/../../3rd/win64/easy_profiler-v2.1.0-msvc22-win64/lib/debug/ -leasy_profiler
} 
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/easy_profiler-2.1.0/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/easy_profiler-2.1.0/Debug/lib -leasy_profiler
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/easy_profiler-2.1.0/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/easy_profiler-2.1.0/Release/lib -leasy_profiler
    }
}
