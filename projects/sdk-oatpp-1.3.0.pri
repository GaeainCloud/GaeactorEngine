

win32: {
        INCLUDEPATH += $$PWD/../../3rd/win64/oatpp-1.3.0/include/oatpp-1.3.0/oatpp
        INCLUDEPATH += $$PWD/../../3rd/win64/oatpp-swagger-1.3.0/include/oatpp-1.3.0/oatpp-swagger
        INCLUDEPATH += $$PWD/../../3rd/win64/oatpp-sqlite-1.3.0/include/oatpp-1.3.0/oatpp-sqlite
        INCLUDEPATH += $$PWD/../../3rd/win64/oatpp-sqlite-1.3.0/include/oatpp-1.3.0/oatpp-sqlite/sqlite
        INCLUDEPATH += $$PWD/../../3rd/win64/oatpp-websocket-1.3.0/include/oatpp-1.3.0/oatpp-websocket

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/oatpp-1.3.0/lib/oatpp-1.3.0/release -loatpp

        LIBS += -L$$PWD/../../3rd/win64/oatpp-swagger-1.3.0/lib/oatpp-1.3.0/release -loatpp-swagger
        LIBS += -lWS2_32

        LIBS += -L$$PWD/../../3rd/win64/oatpp-sqlite-1.3.0/lib/oatpp-1.3.0/release -loatpp-sqlite -lsqlite

        LIBS += -L$$PWD/../../3rd/win64/oatpp-websocket-1.3.0/lib/oatpp-1.3.0/release -loatpp-websocket
    }
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/oatpp-1.3.0/lib/oatpp-1.3.0/debug -loatpp

        LIBS += -L$$PWD/../../3rd/win64/oatpp-swagger-1.3.0/lib/oatpp-1.3.0/debug -loatpp-swagger
        LIBS += -lWS2_32

        LIBS += -L$$PWD/../../3rd/win64/oatpp-sqlite-1.3.0/lib/oatpp-1.3.0/debug -loatpp-sqlite -lsqlite

        LIBS += -L$$PWD/../../3rd/win64/oatpp-websocket-1.3.0/lib/oatpp-1.3.0/debug -loatpp-websocket
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp/Debug/include/oatpp-1.3.0/oatpp
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp-swagger/Debug/include/oatpp-1.3.0/oatpp-swagger
        #INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp-websocket/Debug/include/oatpp-1.3.0/oatpp-websocket
        #INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp-sqlite/Debug/include/oatpp-1.3.0/oatpp-sqlite
        #INCLUDEPATH +=  $$PWD/../../3rd/lnx/sqlite-3430100/debug/include
        LIBS += -L$$PWD/../../3rd/lnx/oatpp/Debug/lib/oatpp-1.3.0 -loatpp
        LIBS += -L$$PWD/../../3rd/lnx/oatpp-swagger/Debug/lib/oatpp-1.3.0 -loatpp-swagger
        #LIBS += -L$$PWD/../../3rd/lnx/oatpp-websocket/Debug/lib/oatpp-1.3.0 -loatpp-websocket
        #LIBS += -L$$PWD/../../3rd/lnx/oatpp-sqlite/Debug/lib/oatpp-1.3.0 -loatpp-sqlite
        #LIBS += -L$$PWD/../../3rd/lnx/sqlite-3430100/debug/lib -lsqlite3
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp/Release/include/oatpp-1.3.0/oatpp
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp-swagger/Release/include/oatpp-1.3.0/oatpp-swagger
        #INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp-websocket/Release/include/oatpp-1.3.0/oatpp-websocket
        #INCLUDEPATH +=  $$PWD/../../3rd/lnx/oatpp-sqlite/Release/include/oatpp-1.3.0/oatpp-sqlite
        #INCLUDEPATH +=  $$PWD/../../3rd/lnx/sqlite-3430100/release/include
        LIBS += -L$$PWD/../../3rd/lnx/oatpp/Release/lib/oatpp-1.3.0 -loatpp
        LIBS += -L$$PWD/../../3rd/lnx/oatpp-swagger/Release/lib/oatpp-1.3.0 -loatpp-swagger
        #LIBS += -L$$PWD/../../3rd/lnx/oatpp-websocket/Release/lib/oatpp-1.3.0 -loatpp-websocket
        #LIBS += -L$$PWD/../../3rd/lnx/oatpp-sqlite/Release/lib/oatpp-1.3.0 -loatpp-sqlite
        #LIBS += -L$$PWD/../../3rd/lnx/sqlite-3430100/release/lib -lsqlite3
    }
}
