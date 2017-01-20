#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T12:17:54
#
#-------------------------------------------------

QT       += core gui opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG -= app_bundle

win32:VERSION = 1.0.0.0 # major.minor.patch.build
else:VERSION = 1.0.0    # major.minor.patch

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TARGET = VisceralFatValidation
TEMPLATE = app

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT QT_MESSAGELOGCONTEXT

SOURCES += main.cpp\
        mainwindow.cpp \
    util.cpp \
    vertex.cpp \
    niftimage.cpp \
    axialslicewidget.cpp \
    commands.cpp \
    opencv.cpp \
    subjectconfig.cpp \
    coronalslicewidget.cpp \
    numerictype.cpp \
    view_axialcoronalhires.cpp \
    view_axialcoronallores.cpp

HEADERS  += mainwindow.h \
    application.h \
    niftimage.h \
    axialslicewidget.h \
    commands.h \
    opencv.h \
    vertex.h \
    exception.h \
    util.h \
    subjectconfig.h \
    coronalslicewidget.h \
    displayinfo.h \
    numerictype.h \
    view_axialcoronalhires.h \
    view_axialcoronallores.h

FORMS    += mainwindow.ui \
    view_axialcoronalhires.ui \
    view_axialcoronallores.ui

DISTFILES += \
    shaders/axialslice.vert \
    shaders/axialslice.frag

# Include the NIFTI files and link the dynamic libraries.
# The DLL files should deploy with the application automatically
INCLUDEPATH += 'D:/DevelLibs/nifticlib-2.0.0/build/include'
DEPENDPATH += 'D:/DevelLibs/nifticlib-2.0.0/build/include'

LIBS += -LD:/DevelLibs/nifticlib-2.0.0/build/lib \
        -lnifticdf \
        -lniftiio \
        -lznz

LIBS += -LD:/DevelLibs/zlib-1.2.8/build/lib \
        -lzlib

INCLUDEPATH += 'D:/DevelLibs/opencv/build/install/include'
DEPENDPATH += 'D:/DevelLibs/opencv/build/install/include'
CONFIG(debug, debug|release): LIBS += -LD:/DevelLibs/opencv/build/lib/Debug \
        -lopencv_core310d \
        -lopencv_imgproc310d \
        -lopencv_highgui310d \
        -lopencv_ml310d \
        -lopencv_video310d

CONFIG(release, debug|release): LIBS += -LD:/DevelLibs/opencv/build/lib/Release \
        -lopencv_core310 \
        -lopencv_imgproc310 \
        -lopencv_highgui310 \
        -lopencv_ml310 \
        -lopencv_video310

RESOURCES += \
    resources.qrc
