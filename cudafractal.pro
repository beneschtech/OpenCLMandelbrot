#-------------------------------------------------
#
# Project created by QtCreator 2018-01-07T13:23:48
#
#-------------------------------------------------

QT       += core gui widgets
CONFIG += c++11

TEMPLATE = app


SOURCES += \
        gpucompute.cpp \
        main.cpp \
        mainwindow.cpp \
        openmpcompute.cpp \
        singlethreadcompute.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

LIBS += \
    -lOpenCL -lgomp

RESOURCES += \
    opencl.qrc

QMAKE_CXXFLAGS += -fopenmp
