win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/google_breakpad/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/google_breakpad/lib/debug \
            -lcommon \
            -lcrash_generation_client \
            -lcrash_generation_server \
            -lexception_handler
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/google_breakpad/lib/release \
            -lcommon \
            -lcrash_generation_client \
            -lcrash_generation_server \
            -lexception_handler
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/breakpad-2023.06.01/debug/include/breakpad
        LIBS += -L$$PWD/../../3rd/lnx/breakpad-2023.06.01/debug/lib \
            -lbreakpad \
            -lbreakpad_client
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/breakpad-2023.06.01/release/include/breakpad
        LIBS += -L$$PWD/../../3rd/lnx/breakpad-2023.06.01/release/lib \
            -lbreakpad \
            -lbreakpad_client
    }
}
