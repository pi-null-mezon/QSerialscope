#------------------------------------------------FFTW---------------------------------------------------------
win32 {

    INCLUDEPATH += $$PWD/../../3rdParties/FFTW
    win32:contains(QMAKE_TARGET.arch, x86_64){

        message( "FFTW library for 64 bit architecture will be used" )
        LIBS += -L$$PWD/../../3rdParties/FFTW/fftw3-64/ -llibfftw3-3

    } else {

        message( "FFTW library for 32 architecture will be used" )
        LIBS += -L$$PWD/../../3rdParties/FFTW/fftw3-32/ -llibfftw3-3

    }
}

raspbian: LIBS += -lfftw3
#-------------------------------------------------------------------------------------------------------------
