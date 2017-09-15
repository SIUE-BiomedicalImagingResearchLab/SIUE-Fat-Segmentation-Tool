#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T12:17:54
#
#-------------------------------------------------

QT       += core gui opengl xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
!macx: CONFIG -= app_bundle

VERSION = 2.0.1.0 # major.minor.patch.build

DEFINES += APP_VERSION=\\\"$$VERSION\\\"

TARGET = "SIUE Fat Segmentation Tool"
TEMPLATE = app

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT QT_MESSAGELOGCONTEXT

SOURCES += main.cpp\
    mainwindow.cpp \
    util.cpp \
    niftimage.cpp \
    axialslicewidget.cpp \
    commands.cpp \
    opencv.cpp \
    subjectconfig.cpp \
    coronalslicewidget.cpp \
    numerictype.cpp \
    view_axialcoronalhires.cpp \
    view_axialcoronallores.cpp \
    tracing.cpp \
    stacktrace.cpp

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
    view_axialcoronallores.h \
    tracing.h \
    stacktrace.h

FORMS    += mainwindow.ui \
    view_axialcoronalhires.ui \
    view_axialcoronallores.ui

RESOURCES += \
    resources.qrc

contains(QT_ARCH, x86_64) {
    message('Building 64-bit binaries')

    exists(customx64.pro): include(customx64.pro)
    else:exists(custom.pro): include(custom.pro)
    else:exists(customx86.pro): include(customx86.pro)
} else:contains(QT_ARCH, i386) {
    message('Building 32-bit binaries')
    exists(customx86.pro): include(customx86.pro)
    else:exists(custom.pro): include(custom.pro)
}
