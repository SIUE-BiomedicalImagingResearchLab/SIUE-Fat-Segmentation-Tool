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
    QVector4D getLocation();
    QVector4D transformLocation(QVector4D location);

    void setImages(NIFTImage *fat, NIFTImage *water);
    bool isLoaded();

    SliceDisplayType getDisplayType();
    void setDisplayType(SliceDisplayType type);

    float getContrast();
    void setContrast(float contrast);

    float getBrightness();
    void setBrightness(float brightness);

    void resetView();

    float &rscaling();
    QVector3D &rtranslation();

    QMatrix4x4 getMVP();

    void setUndoStack(QUndoStack *stack);

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
