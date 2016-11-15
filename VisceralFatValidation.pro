#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T12:17:54
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VisceralFatValidation
TEMPLATE = app

release: DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

SOURCES += main.cpp\
        mainwindow.cpp \
    util.cpp \
    axialfatslicewidget.cpp \
    nifti.cpp \
    vertex.cpp

HEADERS  += mainwindow.h \
    util.hpp \
    axialfatslicewidget.h \
    exception.hpp \
    nifti.hpp \
    vertex.hpp

FORMS    += mainwindow.ui

DISTFILES += \
    shaders/axialslice.vert \
    shaders/axialslice.frag

# Include the NIFTI files and link the dynamic libraries.
# The DLL files should deploy with the application automatically
INCLUDEPATH += $$PWD/nifti/include
DEPENDPATH += $$PWD/nifti/include

LIBS += -L$$PWD/nifti/lib/ \
        libnifticdf.dll.a \
        libniftiio.dll.a \
        libznz.dll.a

RESOURCES += \
    resources.qrc
