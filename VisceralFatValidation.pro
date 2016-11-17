#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T12:17:54
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VisceralFatValidation
TEMPLATE = app

CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

SOURCES += main.cpp\
        mainwindow.cpp \
    util.cpp \
    vertex.cpp \
    niftimage.cpp \
    axialslicewidget.cpp

HEADERS  += mainwindow.h \
    util.hpp \
    exception.hpp \
    vertex.hpp \
    application.h \
    niftimage.h \
    axialslicewidget.h

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

INCLUDEPATH += 'C:/Program Files/opencv2/include'
DEPENDPATH += 'C:/Program Files/opencv2/include'
LIBS += -L'C:/Program Files/opencv2/lib' \
        -lopencv_world310

RESOURCES += \
    resources.qrc
