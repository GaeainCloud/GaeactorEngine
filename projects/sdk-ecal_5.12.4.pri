
win32: {
    DEFINES += PROTOBUF_USE_DLLS
        INCLUDEPATH += $$PWD/../../3rd/win64/boost-1.84.0/include/boost-1_84
        INCLUDEPATH += $$PWD/../../3rd/win64/msgpack-c-cpp-6.1.1/include
        INCLUDEPATH += $$PWD/../../3rd/win64/ecal-5.12.4/include

CONFIG(release, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/ecal-5.12.4/lib/release -lecal_core_c \
    -lecal_core \
    -lecal_ecaltime_pb \
    -lecal_mon_plugin_lib \
    -lecal_proto \
    -lhdf5 \
    -lhdf5_cpp \
    -llibprotobuf-lite \
    -llibprotoc \
    -lecal_app_pb \
    -lecal_core_pb \
    -lecal_ecal-utils \
    -lecal_hdf5 \
    -lecal_pb \
    -lecal_rec_addon_core \
    -llibprotobuf 
}

CONFIG(debug, debug|release): {
    LIBS += -L$$PWD/../../3rd/win64/ecal-5.12.4/lib/debug -lecal_core_cd \
    -lecal_cored \
    -lecal_ecaltime_pbd \
    -lecal_mon_plugin_libd \
    -lecal_protod \
    -lhdf5_D \
    -lhdf5_cpp_D \
    -llibprotobuf-lited \
    -llibprotocd \
    -lecal_app_pbd \
    -lecal_core_pbd \
    -lecal_ecal-utilsd \
    -lecal_hdf5d \
    -lecal_pbd \
    -lecal_rec_addon_cored \
    -llibprotobufd 
}
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/ecal-5.12.4/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/ecal-5.12.4/Debug/lib -lecal_core_c \
        -lecal_core \
        -lecal_ecaltime_pb \
        -lecal_mon_plugin_lib \
        -lecal_proto \
        -lprotobuf-lited \
        -lprotocd \
        -lecal_app_pb \
        -lecal_core_pb \
        -lecal_ecal-utils \
        -lecal_hdf5 \
        -lecal_pb \
        -lecal_rec_addon_core \
        -lprotobufd\
        -lhdf5_debug \
        -lhdf5_cpp_debug \
        -ltcp_pubsub
        }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/ecal-5.12.4/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/ecal-5.12.4/Release/lib -lecal_core_c \
        -lecal_core \
        -lecal_ecaltime_pb \
        -lecal_mon_plugin_lib \
        -lecal_proto \
        -lprotobuf-lite \
        -lprotoc \
        -lecal_app_pb \
        -lecal_core_pb \
        -lecal_ecal-utils \
        -lecal_hdf5 \
        -lecal_pb \
        -lecal_rec_addon_core \
        -lprotobuf\
        -lhdf5 \
        -lhdf5_cpp \
        -ltcp_pubsub
    }
}
