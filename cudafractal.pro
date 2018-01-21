#-------------------------------------------------
#
# Project created by QtCreator 2018-01-07T13:23:48
#
#-------------------------------------------------

QT       += core gui widgets
CONFIG += c++11

TARGET = cudafractal
TEMPLATE = app


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    cpufractalcomputethread.cpp \
    gpukernel.cpp

HEADERS += \
        mainwindow.h \
    cpufractalcomputethread.h

FORMS += \
        mainwindow.ui

LIBS += \
    -lOpenCL
