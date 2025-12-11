win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/libevent/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/libevent/lib/Debug \
            -levent \
            -levent_core\
            -levent_extra\
            -levent_openssl
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/libevent/lib/Release \
            -levent \
            -levent_core\
            -levent_extra\
            -levent_openssl
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/libevent-2.1.12/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/libevent-2.1.12/Debug/lib \
            -levent \
            -levent_core\
            -levent_extra\
            -levent_openssl
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/libevent-2.1.12/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/libevent-2.1.12/Release/lib \
            -levent \
            -levent_core\
            -levent_extra\
            -levent_openssl

    }
}
