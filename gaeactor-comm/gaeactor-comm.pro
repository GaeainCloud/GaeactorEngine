QT = core

CONFIG += c++17

TARGET = gaeactor-comm
DEFINES += GAEACTOR_COMM_LIBRARY
CONFIG += force_debug_info

TEMPLATE = lib
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += BUILD_WITH_EASY_PROFILER

include($$_PRO_FILE_PWD_/../projects/dynamic-lib-install.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-gaeactor-utils.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-ecal_5.12.4.pri)
include($$_PRO_FILE_PWD_/../projects/protobufdefine.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-easyprofiler.pri)
include($$_PRO_FILE_PWD_/../projects/sdk-libuv.pri)


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    Async.hpp \
    EventLoop.hpp \
    TcpAcceptor.hpp \
    TcpConnection.hpp \
    Timer.hpp \
    TimerWheel.hpp \
    gaeactor_comm_commbase.h \
    gaeactor_comm_define.h \
    gaeactor_comm_global.h \
    gaeactor_comm_interface.h \
    gaeactor_comm_processor.h \
    $$_PRO_FILE_PWD_/src/gaeactor_comm_base.h \
    gaeactor_comm_socketaddr.h \
    gaeactor_comm_tcp_client.h \
    gaeactor_comm_tcp_server.hpp \
    gaeactor_comm_udp.h \
    src/publish/gaeactor_comm_publish_binary.h \
    src/publish/gaeactor_comm_publish_protobuf.h \
    src/publish/gaeactor_comm_publish_struct.h \
    src/publish/gaeactor_comm_publish_string.h \
    src/subscrib/gaeactor_comm_subscribe_binary.h \
    src/subscrib/gaeactor_comm_subscribe_protobuf.h \
    src/subscrib/gaeactor_comm_subscribe_string.h \
    src/subscrib/gaeactor_comm_subscribe_struct.h

SOURCES += \
    Async.cpp \
    EventLoop.cpp \
    TcpAcceptor.cpp \
    TcpConnection.cpp \
    Timer.cpp \
    gaeactor_comm_commbase.cpp \
    gaeactor_comm_interface.cpp \
    gaeactor_comm_processor.cpp \
    $$_PRO_FILE_PWD_/src/gaeactor_comm_base.cpp \
    gaeactor_comm_socketaddr.cpp \
    gaeactor_comm_tcp_client.cpp \
    gaeactor_comm_tcp_server.cpp \
    gaeactor_comm_udp.cpp \
    src/publish/gaeactor_comm_publish_binary.cpp \
    src/publish/gaeactor_comm_publish_string.cpp \
    src/subscrib/gaeactor_comm_subscribe_binary.cpp \
    src/subscrib/gaeactor_comm_subscribe_string.cpp




copyDynamicLibrary(gaeactor-comm)
