#ifndef AXIALFATSLICEWIDGET_H
#define AXIALFATSLICEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QVector>

#include <nifti/include/nifti1.h>
#include <nifti/include/fslio.h>

#include "nifti.hpp"
#include "vertex.hpp"

enum AxialDisplayType
{
    FatOnly,
    WaterOnly,
    FatFraction,
    WaterFraction
};

class AxialFatSliceWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
private:
    AxialDisplayType displayType;

    nifti_image *fatUpperImage;
    nifti_image *fatLowerImage;
    nifti_image *waterUpperImage;
    nifti_image *waterLowerImage;

    QOpenGLShaderProgram *program;
    GLuint vertexBuf, indexBuf;
    GLuint vertexObject;
    GLuint texture;
    QVector<VertexPT> sliceVertices;
    QVector<unsigned short> sliceIndices;

    int currSlice;
    int currTime;

public:
    AxialFatSliceWidget(QWidget *parent);
    ~AxialFatSliceWidget();

    void SetAxialSlice(int slice, int time = -1);

    void setImages(nifti_image *fatUpper, nifti_image *fatLower, nifti_image *waterUpper, nifti_image *waterLower);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mouseMoveEvent(QMouseEvent *eventMove);
    void mousePressEvent(QMouseEvent *eventPress);
    void mouseReleaseEvent(QMouseEvent *eventRelease);
};

#endif // AXIALFATSLICEWIDGET_H
