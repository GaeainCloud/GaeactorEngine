win32 {
    INCLUDEPATH += $$PWD/../../3rd/win64/fastrtps_2.10.1/include

    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/fastrtps_2.10.1/lib/x64Win64VS2019 -lfastrtpsd-2.10 -lfoonathan_memory-0.7.3-dbg
#        LIBS += -L$$(FASTRTPS_ROOT)\lib\x64Win64VS2019 -lfastcdrd-1.0 -lfastrtpsd-2.10 -lfoonathan_memory-0.7.3-dbg -llibfastcdrd-1.0 -llibfastrtpsd-2.10
        LIBS += -lAdvapi32
    }

    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/win64/fastrtps_2.10.1/lib/x64Win64VS2019 -lfastrtps-2.10 -lfoonathan_memory-0.7.3
        LIBS += -lAdvapi32
    }
}

unix {
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/Fast-CDR-1.1.1/Debug/include
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/memory-0.7-3/Debug/include
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/Fast-DDS-2.10.2/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/Fast-CDR-1.1.1/Debug/lib -lfastcdr
        LIBS += -L$$PWD/../../3rd/lnx/memory-0.7-3/Debug/lib -lfoonathan_memory-0.7.3-dbg
        LIBS += -L$$PWD/../../3rd/lnx/Fast-DDS-2.10.2/Debug/lib -lfastrtps
    }
    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/Fast-CDR-1.1.1/Release/include
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/memory-0.7-3/Release/include
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/Fast-DDS-2.10.2/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/Fast-CDR-1.1.1/Release/lib -lfastcdr
        LIBS += -L$$PWD/../../3rd/lnx/memory-0.7-3/Release/lib -lfoonathan_memory-0.7.3
        LIBS += -L$$PWD/../../3rd/lnx/Fast-DDS-2.10.2/Release/lib -lfastrtps
    }
}
