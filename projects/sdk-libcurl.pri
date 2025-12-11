win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/curl-8.4.0/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/curl-8.4.0/lib/Debug \
            -llibcurl-d_imp 
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/curl-8.4.0/lib/Release \
            -llibcurl_imp 
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/curl-8.4.0/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/curl-8.4.0/Debug/lib \
            -lcurl-d
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/curl-8.4.0/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/curl-8.4.0/Release/lib \
            -lcurl
    }
}
