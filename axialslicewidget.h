#ifndef AXIALSLICEWIDGET_H
#define AXIALSLICEWIDGET_H

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

#include <opencv2/opencv.hpp>

#include "niftimage.h"
#include "vertex.hpp"

// TODO: These three items outside of the widget class may not belong here.
// In reality, the Minimap widget might use these too and a common place for them would be nice
enum AxialDisplayType
{
    FatOnly,
    WaterOnly,
    FatFraction,
    WaterFraction
};

enum ColorMap
{
    Autumn = 0,
    Bone,
    Cool,
    Copper,
    Gray,
    Hot,
    HSV,
    Jet,
    Parula,
    Pink,
    Spring,
    Summer,
    Winter,
    Count
};

static QString colorMapFragName[ColorMap::Count] =
{
    ":/shaders/MATLAB_autumn.frag",
    ":/shaders/MATLAB_bone.frag",
    ":/shaders/MATLAB_cool.frag",
    ":/shaders/MATLAB_copper.frag",
    ":/shaders/MATLAB_gray.frag",
    ":/shaders/MATLAB_hot.frag",
    ":/shaders/MATLAB_hsv.frag",
    ":/shaders/MATLAB_jet.frag",
    ":/shaders/MATLAB_parula.frag",
    ":/shaders/MATLAB_pink.frag",
    ":/shaders/MATLAB_spring.frag",
    ":/shaders/MATLAB_summer.frag",
    ":/shaders/MATLAB_winter.frag"
};

class AxialSliceWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
private:
    AxialDisplayType displayType;

    NIFTImage *fatImage;
    NIFTImage *waterImage;

    QOpenGLShaderProgram *program[ColorMap::Count];
    GLuint vertexBuf, indexBuf;
    GLuint vertexObject;
    GLuint texture;
    QVector<VertexPT> sliceVertices;
    QVector<unsigned short> sliceIndices;

    int curSlice;
    int curTime;

    ColorMap curColorMap;

    float curBrightness;
    float curContrast;

public:
    AxialSliceWidget(QWidget *parent);
    ~AxialSliceWidget();

    void setAxialSlice(int slice, int time = -1);

    void setImages(NIFTImage *fat, NIFTImage *water);

    AxialDisplayType getDisplayType();
    void setDisplayType(AxialDisplayType type);

    ColorMap getColorMap();
    void setColorMap(ColorMap map);

    float getContrast();
    void setContrast(float contrast);

    float getBrightness();
    void setBrightness(float brightness);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mouseMoveEvent(QMouseEvent *eventMove);
    void mousePressEvent(QMouseEvent *eventPress);
    void mouseReleaseEvent(QMouseEvent *eventRelease);
};

#endif // AXIALSLICEWIDGET_H
