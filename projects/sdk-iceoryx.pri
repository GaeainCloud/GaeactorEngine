win32: {

DEFINES += WIN32_LEAN_AND_MEAN
    INCLUDEPATH += $$PWD/../../3rd/win64/iceoryx-2.0.3/build_bin/include/iceoryx/v
CONFIG(release, debug|release):{
    LIBS += -L$$PWD/../../3rd/win64/iceoryx-2.0.3/build_bin/lib/release -liceoryx_hoofs -liceoryx_posh -liceoryx_binding_c -liceoryx_platform -liceoryx_posh_config -liceoryx_posh_gateway -liceoryx_posh_roudi
}
CONFIG(debug, debug|release):{
    LIBS += -L$$PWD/../../3rd/win64/iceoryx-2.0.3/build_bin/lib/debug -liceoryx_hoofs -liceoryx_posh -liceoryx_binding_c -liceoryx_platform -liceoryx_posh_config -liceoryx_posh_gateway -liceoryx_posh_roudi
}

}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/iceoryx-2.0.3/Debug/include/iceoryx/v
        LIBS += -L$$PWD/../../3rd/lnx/iceoryx-2.0.3/Debug/lib -liceoryx_hoofs -liceoryx_posh -liceoryx_binding_c -liceoryx_platform -liceoryx_posh_config -liceoryx_posh_gateway -liceoryx_posh_roudi
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/iceoryx-2.0.3/Release/include/iceoryx/v
        LIBS += -L$$PWD/../../3rd/lnx/iceoryx-2.0.3/Release/lib -liceoryx_hoofs -liceoryx_posh -liceoryx_binding_c -liceoryx_platform -liceoryx_posh_config -liceoryx_posh_gateway -liceoryx_posh_roudi
    }
}
