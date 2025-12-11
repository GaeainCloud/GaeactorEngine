win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/openssl1.1.1u/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/openssl1.1.1u/lib/debug/ -llibcrypto
        LIBS += -L$$PWD/../../3rd/win64/openssl1.1.1u/lib/debug/ -llibssl
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/openssl1.1.1u/lib/release/ -llibcrypto
        LIBS += -L$$PWD/../../3rd/win64/openssl1.1.1u/lib/release/ -llibssl
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/openssl-1.1.1u/debug/include
        LIBS += -L$$PWD/../../3rd/lnx/openssl-1.1.1u/debug/lib -lcrypto -lssl
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/openssl-1.1.1u/release/include
        LIBS += -L$$PWD/../../3rd/lnx/openssl-1.1.1u/release/lib -lcrypto -lssl
    }
}
