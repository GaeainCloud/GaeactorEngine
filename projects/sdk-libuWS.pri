win32: {
    INCLUDEPATH += $$PWD/../../3rd/win64/uWS/include
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/uWS/lib/Debug \
            -luWS
    }
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/uWS/lib/Release \
            -luWS
    }
}
unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/uWebSockets-0.14/debug/include
#        LIBS += -L$$PWD/../../3rd/lnx/uWebSockets-0.14/debug/lib \
#            -luv
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/uWebSockets-0.14/release/include
#        LIBS += -L$$PWD/../../3rd/lnx/uWebSockets-0.14/uWebSockets-0.14/lib \
#            -luv

    }

INCLUDEPATH +=  $$PWD/../../3rd/lnx/uWebSockets-0.14/src

SOURCES += \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Group.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Node.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Extensions.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Hub.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Networking.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Epoll.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Socket.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/WebSocket.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/HTTPSocket.cpp \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Room.cpp

HEADERS += \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Asio.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Epoll.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/HTTPSocket.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Libuv.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Socket.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/WebSocket.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Backend.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Group.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Node.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/uWS.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/WebSocketProtocol.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Extensions.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Hub.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Networking.h \
    $$PWD/../../3rd/lnx/uWebSockets-0.14/src/Room.h

}
