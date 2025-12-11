
include($$PWD/sdk-libWebsocketpp-boost-1_84_0.pri)

win32: {
INCLUDEPATH += $$PWD/../../3rd/win64/websocketpp-0.8.2/include
}

unix {
INCLUDEPATH += $$PWD/../../3rd/lnx/websocketpp-0.8.2/include
}
