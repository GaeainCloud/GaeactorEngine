
DEFINES += SQLITE_ENABLE_MATH_FUNCTIONS

DEFINES += SQLITE_ENABLE_JSON1
win32 {
    INCLUDEPATH += \
        $$PWD/../../3rd/win64/sqlite_orm-master/include \
        $$PWD/../../3rd/win64/sqlite-amalgamation-3450000 \
        $$PWD/../../3rd/win64/sqlite3/include

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/sqlite3/lib/debug -lsqlite
}

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/sqlite3/lib/release -lsqlite
}
}

unix {
INCLUDEPATH += \
    $$PWD/../../3rd/lnx/sqlite_orm-1.8.2/include \

    CONFIG(debug, debug|release): {
    INCLUDEPATH += $$PWD/../../3rd/lnx/sqlite-3430100/debug/include
    LIBS += -L$$PWD/../../3rd/lnx/sqlite-3430100/debug/lib -lsqlite3
    }

    CONFIG(release, debug|release): {
    INCLUDEPATH += $$PWD/../../3rd/lnx/sqlite-3430100/release/include
    LIBS += -L$$PWD/../../3rd/lnx/sqlite-3430100/release/lib -lsqlite3
    }
}
