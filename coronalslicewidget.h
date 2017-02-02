#ifndef CORONALSLICEWIDGET_H
#define CORONALSLICEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QDebug>
#include <QMouseEvent>
#include <QWidget>
#include <QVector>
#include <QVector4D>
#include <QMatrix4x4>
#include <QUndoStack>
#include <QPainter>

#include <nifti/nifti1.h>
#include <nifti/nifti1_io.h>

#include <opencv2/opencv.hpp>

#include "niftimage.h"
#include "vertex.h"
#include "commands.h"
#include "displayinfo.h"

class CoronalSliceWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

private:
    SliceDisplayType displayType;

    NIFTImage *fatImage;
    NIFTImage *waterImage;

    // Each bit represents whether the specified item in Dirty enum needs to be updated on drawing
    int dirty;

    QOpenGLShaderProgram *program;
    GLuint sliceVertexBuf, sliceIndexBuf;
    GLuint sliceVertexObject;
    GLuint sliceTexture;
    QVector<VertexPT> sliceVertices;
    QVector<unsigned short> sliceIndices;

    QPoint lineStart, lineEnd;
    int lineWidth;

    // Location of where the user is viewing.
    // The format is (X, Y, Z, T) where T is time
    QVector4D location;

    bool startPan;
    QPoint lastMousePos;
    CommandID moveID;

    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;

    float scaling;
    QVector3D translation;

    QUndoStack *undoStack;

public:
    CoronalSliceWidget(QWidget *parent);
    ~CoronalSliceWidget();

    // A value of Location::NoChange(-1) means no change for the location
    void setLocation(QVector4D location);
    QVector4D getLocation() const;
    QVector4D transformLocation(QVector4D location) const;

    void setup(NIFTImage *fat, NIFTImage *water);
    bool isLoaded() const;

    SliceDisplayType getDisplayType() const;
    void setDisplayType(SliceDisplayType type);

    void resetView();

    float &rscaling();
    QVector3D &rtranslation();

    QMatrix4x4 getMVPMatrix() const;

    QMatrix4x4 getWindowToNIFTIMatrix(bool includeMVP = true) const;
    QMatrix4x4 getWindowToOpenGLMatrix(bool includeMVP = true, bool flipY = true) const;
    QMatrix4x4 getNIFTIToOpenGLMatrix(bool includeMVP = true, bool flipY = true) const;

    void setUndoStack(QUndoStack *stack);

    void setDirty(Dirty bit);

    void updateTexture();
    void updateCrosshairLine();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void initializeSliceView();
    void initializeCrosshairLine();

    void mouseMoveEvent(QMouseEvent *eventMove);
    void mousePressEvent(QMouseEvent *eventPress);
    void mouseReleaseEvent(QMouseEvent *eventRelease);

    void wheelEvent(QWheelEvent *event);
};

#endif // CORONALSLICEWIDGET_H
