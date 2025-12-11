

win32 : {
CONFIG(debug, debug|release):           LIBS += -L$$PWD/../../3rd/win64/clapack/win64 -lblasd
else: CONFIG(release, debug|release):   LIBS += -L$$PWD/../../3rd/win64/clapack/win64 -lblas
CONFIG(debug, debug|release):           LIBS += -L$$PWD/../../3rd/win64/clapack/win64 -llibf2cd
else: CONFIG(release, debug|release):   LIBS += -L$$PWD/../../3rd/win64/clapack/win64 -llibf2c
CONFIG(debug, debug|release):           LIBS += -L$$PWD/../../3rd/win64/clapack/win64 -llapackd
else: CONFIG(release, debug|release):   LIBS += -L$$PWD/../../3rd/win64/clapack/win64 -llapack
}

macx : {

}

unix {
    CONFIG(debug, debug|release): {
        LIBS += -L$$PWD/../../3rd/lnx/lapack-3.12.0/Debug/lib -lblasd
        LIBS += -L$$PWD/../../3rd/lnx/lapack-3.12.0/Debug/lib -llapackd
        LIBS += -lgfortran
    }
    
    CONFIG(release, debug|release): {
        LIBS += -L$$PWD/../../3rd/lnx/lapack-3.12.0/Release/lib -lblas
        LIBS += -L$$PWD/../../3rd/lnx/lapack-3.12.0/Release/lib -llapack
        LIBS += -lgfortran
    }
}
