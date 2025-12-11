include($$PWD/sdk-clapack.pri)
win32: {

    #DEFINES += ARMA_USE_FFTW3
    INCLUDEPATH += $$PWD/../../3rd/win64/armadillo-12.6.2/include

    #CONFIG(debug, debug|release): {
    #    LIBS += -L$$PWD/../3rd/fftw-3.3.5-dll64 -llibfftw3-3
    #    LIBS += -L$$PWD/../3rd/fftw-3.3.5-dll64 -llibfftw3f-3
    #    LIBS += -L$$PWD/../3rd/fftw-3.3.5-dll64 -llibfftw3l-3
    #}

    #CONFIG(release, debug|release): {
    #    LIBS += -L$$PWD/../3rd/fftw-3.3.5-dll64 -llibfftw3-3
    #    LIBS += -L$$PWD/../3rd/fftw-3.3.5-dll64 -llibfftw3f-3
    #    LIBS += -L$$PWD/../3rd/fftw-3.3.5-dll64 -llibfftw3l-3
    #}

}

unix {
    #DEFINES += ARMA_USE_FFTW3
    ## DEFINES += ARMA_USE_LAPACK
    ## DEFINES += ARMA_USE_BLAS
    CONFIG(debug, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/armadillo-12.6.2/Debug/include
        LIBS += -L$$PWD/../../3rd/lnx/armadillo-12.6.2/Debug/lib -larmadillo
    }

    CONFIG(release, debug|release): {
        INCLUDEPATH +=  $$PWD/../../3rd/lnx/armadillo-12.6.2/Release/include
        LIBS += -L$$PWD/../../3rd/lnx/armadillo-12.6.2/Release/lib -larmadillo
    }

#    CONFIG(debug, debug|release): {
#        INCLUDEPATH +=  $$PWD/../../3rd/lnx/fftw-3.3.5/debug/include
#        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.5/debug/lib -lfftw3
#        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.5/debug/lib -lfftw3f
#        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.5/debug/lib -lfftw3l
#    }

#    CONFIG(release, debug|release): {
#        INCLUDEPATH +=  $$PWD/../../3rd/lnx/fftw-3.3.5/release/include
#        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.5/release/lib -lfftw3
#        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.5/release/lib -lfftw3f
#        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.5/release/lib -lfftw3l
#    }

##    CONFIG(debug, debug|release): {
##        INCLUDEPATH +=  $$PWD/../../3rd/lnx/fftw-3.3.10/debug/include
##        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.10/debug/lib -lfftw3
##        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.10/debug/lib -lfftw3f
##        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.10/debug/lib -lfftw3l
##    }

##    CONFIG(release, debug|release): {
##        INCLUDEPATH +=  $$PWD/../../3rd/lnx/fftw-3.3.10/release/include
##        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.10/release/lib -lfftw3
##        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.10/release/lib -lfftw3f
##        LIBS += -L$$PWD/../../3rd/lnx/fftw-3.3.10/release/lib -lfftw3l
##    }
}
