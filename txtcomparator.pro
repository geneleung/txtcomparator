#-------------------------------------------------
#
# Project created by QtCreator 2014-03-08T20:23:27
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = txtcomparator
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH  += include \


SOURCES += main.cpp \
    utils/RefBase.cpp \
    test/testdatatypes.cpp \
    test/testSP.cpp

HEADERS += \
    include/stddefs.h \
    include/support/datatypes.h \
    include/utils/atomic-x86.h \
    include/utils/atomic.h \
    include/utils/log.h \
    include/utils/RefBase.h \
    include/utils/StrongPointer.h
