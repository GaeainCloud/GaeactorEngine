win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/redis-plus-plus-1.3.12/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/redis-plus-plus-1.3.12/lib/debug \
            -lredis++ 
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/redis-plus-plus-1.3.12/lib/release \
            -lredis++ 
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/redis-plus-plus-1.3.12/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/redis-plus-plus-1.3.12/Debug/lib -lredis++
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/redis-plus-plus-1.3.12/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/redis-plus-plus-1.3.12/Release/lib -lredis++
    }
}
