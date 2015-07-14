#-------------------------------------------------
#
# Project created by QtCreator 2014-05-30T10:49:21
#
#-------------------------------------------------

QT       += core \
            gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#--------------------CONFIG------------------------
!win32 {

    CONFIG +=   raspbian \
                serialport
    message("Build on raspberry for raspberry")

} else {

    QT += serialport

}

CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT \
                                           QT_NO_DEBUG_STREAM


#------------------App Info-----------------------
TEMPLATE = app
TARGET = adc2pc
VERSION = 1.0.0.1
DEFINES +=  APP_NAME=\\\"$${TARGET}\\\" \
            APP_VERSION=\\\"$${VERSION}\\\"

#--------------------Sources----------------------
SOURCES +=  main.cpp\
            mainwindow.cpp \
            qserialprocessor.cpp \
            qharmonicprocessor.cpp \
            qtransmissiondialog.cpp

HEADERS  += mainwindow.h \
            about.h \
            qserialprocessor.h \
            qharmonicprocessor.h \
            qtransmissiondialog.h

#------------------3rd parties---------------------
include(FFTW.pri)
include(QEasyPlot.pri)

#-------------_----Forms & icons-------------------
FORMS +=    qtransmissiondialog.ui
RC_ICONS = $$PWD/../Resources/adc2pc.ico


#------------------Additional----------------------
win32-g++|raspbian: DEFINES += DEFINE_USE_CMATH

