#-------------------------------------------------
#
# Project created by QtCreator 2016-10-26T12:17:54
#
#-------------------------------------------------

QT       += core gui opengl xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG -= app_bundle

win32:VERSION = 1.0.1.0 # major.minor.patch.build
else:VERSION = 1.0.1    # major.minor.patch

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
    view_axialcoronallores.cpp \
    tracing.cpp \
    view_3D.cpp \
    threedwidget.cpp

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
    view_3D.h \
    threedwidget.h

FORMS    += mainwindow.ui \
    view_axialcoronalhires.ui \
    view_axialcoronallores.ui \
    view_3D.ui

RESOURCES += \
    resources.qrc

exists(custom.pro): include(custom.pro)

DISTFILES +=
