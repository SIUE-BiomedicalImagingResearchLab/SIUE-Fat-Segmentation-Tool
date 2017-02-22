#ifndef THREEDWIDGET_H
#define THREEDWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QVector>
#include <QVector4D>
#include <QMatrix4x4>
#include <QUndoStack>
#include <QPainter>
#include <array>
#include <chrono>
#include <QMessageBox>
#include <QLabel>

#include <nifti/nifti1.h>
#include <nifti/nifti1_io.h>

#include <opencv2/opencv.hpp>

#include "niftimage.h"
#include "vertex.h"
#include "commands.h"
#include "tracing.h"
#include "displayinfo.h"

class ThreeDWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

private:
    NIFTImage *fatImage;
    NIFTImage *waterImage;
    TracingData *tracingData;

    // Each bit represents whether the specified item in Dirty enum needs to be updated on drawing
    int dirty;

    QOpenGLShaderProgram *sliceProgram;
    GLuint sliceVertexBuf, sliceIndexBuf;
    GLuint sliceVertexObject;
    GLuint slicePrimTexture;
    GLuint sliceSecdTexture;
    QVector<VertexPT> sliceVertices;
    QVector<unsigned short> sliceIndices;
    bool sliceTexturePrimInit;
    bool sliceTextureSecdInit;

    QOpenGLShaderProgram *traceProgram;
    GLuint traceVertexBuf, traceIndexBuf;
    GLuint traceVertexObject;
    GLuint traceTextures[(int)TracingLayer::Count];
    QVector<VertexPT> traceVertices;
    QVector<unsigned short> traceIndices;
    std::array<bool, (int)TracingLayer::Count> traceTextureInit;

    GLuint colorMapTexture[ColorMap::Count];

    // Location of where the user is viewing.
    // The format is (X, Y, Z, T) where T is time
    QVector4D location;

    QLabel *locationLabel;

    ColorMap primColorMap;
    float primOpacity;

    ColorMap secdColorMap;
    float secdOpacity;

    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;

    TracingLayer tracingLayer;
    std::array<bool, (size_t)TracingLayer::Count> tracingLayerVisible;

    QUndoStack *undoStack;

public:
    ThreeDWidget();
};

#endif // THREEDWIDGET_H
