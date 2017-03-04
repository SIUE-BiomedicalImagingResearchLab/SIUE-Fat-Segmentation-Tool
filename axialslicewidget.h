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
#include <QVector4D>
#include <QMatrix4x4>
#include <QUndoStack>
#include <QPainter>
#include <array>
#include <chrono>
#include <QMessageBox>
#include <QLabel>
#include <QTime>
#include <QElapsedTimer>

#include <nifti/nifti1.h>
#include <nifti/nifti1_io.h>

#include <opencv2/opencv.hpp>

#include "niftimage.h"
#include "vertex.h"
#include "commands.h"
#include "tracing.h"
#include "displayinfo.h"

struct FatLayer
{
    cv::Mat points;
    std::vector<QTime> drawingTime;
};


struct FatLayerSlice
{
    QTime drawingTime;
    std::vector<QPointF> points;

    FatLayerSlice() : drawingTime(), points() {}
};

class AxialSliceWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

private:
    SliceDisplayType displayType;

    NIFTImage *fatImage;
    NIFTImage *waterImage;
    TracingData *tracingData;

    std::array<QColor, (size_t)TracingLayer::Count> tracingLayerColors;
    TracingCommand *mouseCommand;

    // Each bit represents whether the specified item in Dirty enum needs to be updated on drawing
    int dirty;

    QOpenGLShaderProgram *sliceProgram;
    GLuint sliceVertexBuf, sliceIndexBuf;
    //GLuint sliceVertexObject;
    QOpenGLVertexArrayObject sliceVertexObject;
    GLuint slicePrimTexture;
    GLuint sliceSecdTexture;
    QVector<VertexPT> sliceVertices;
    QVector<unsigned short> sliceIndices;
    bool sliceTexturePrimInit;
    bool sliceTextureSecdInit;

    QOpenGLShaderProgram *traceProgram;
    GLuint traceVertexBuf, traceIndexBuf;
    //GLuint traceVertexObject;
    QOpenGLVertexArrayObject traceVertexObject;
    GLuint traceTextures[(int)TracingLayer::Count];
    QVector<VertexPT> traceVertices;
    QVector<unsigned short> traceIndices;
    std::array<bool, (int)TracingLayer::Count> traceTextureInit;

    GLuint colorMapTexture[(int)ColorMap::Count];

    // Location of where the user is viewing.
    // The format is (X, Y, Z, T) where T is time
    QVector4D location;

    QLabel *locationLabel;

    ColorMap primColorMap;
    float primOpacity;

    ColorMap secdColorMap;
    float secdOpacity;

    float brightness;
    float brightnessThreshold;
    float contrast;

    // Sets whether drawing or erasing...useful if new draw modes are added like drawing lines
    DrawMode drawMode;
    bool startDraw;
    QElapsedTimer drawTimer;

    int eraserBrushWidth;

    bool startPan;
    QPoint lastMousePos;
    QPoint lastMousePosNIFTI;
    CommandID moveID;

    QMatrix4x4 projectionMatrix;
    QMatrix4x4 viewMatrix;

    float scaling;
    QVector3D translation;

    TracingLayer tracingLayer;
    std::array<bool, (size_t)TracingLayer::Count> tracingLayerVisible;

    QElapsedTimer fpsTimer;
    int frameCount;
    float fps;

    QUndoStack *undoStack;

public:
    AxialSliceWidget(QWidget *parent);
    ~AxialSliceWidget();

    void readSettings(QSettings &settings);
    void writeSettings(QSettings &settings);

    // A value of Location::NoChange(-1) means no change for the location
    void setLocation(QVector4D location);
    QVector4D getLocation() const;
    QVector4D transformLocation(QVector4D location) const;

    QLabel *getLocationLabel() const;
    void setLocationLabel(QLabel *label);

    void setup(NIFTImage *fat, NIFTImage *water, TracingData *tracing);
    bool isLoaded() const;

    // Performs actions when a new image is loaded
    void imageLoaded();

    SliceDisplayType getDisplayType() const;
    void setDisplayType(SliceDisplayType type);

    ColorMap getPrimColorMap() const;
    void setPrimColorMap(ColorMap map);

    float getPrimOpacity() const;
    void setPrimOpacity(float opacity);

    ColorMap getSecdColorMap() const;
    void setSecdColorMap(ColorMap map);

    float getSecdOpacity() const;
    void setSecdOpacity(float opacity);

    float getBrightness() const;
    void setBrightness(float brightness);

    float getBrightnessThreshold() const;
    void setBrightnessThreshold(float threshold);

    float getContrast() const;
    void setContrast(float contrast);

    DrawMode getDrawMode() const;
    void setDrawMode(DrawMode mode);

    int getEraserBrushWidth() const;
    void setEraserBrushWidth(int width);

    TracingLayer getTracingLayer() const;
    void setTracingLayer(TracingLayer layer);

    bool getTracingLayerVisible(TracingLayer layer) const;
    void setTracingLayerVisible(TracingLayer layer, bool value);

    TracingLayerData &getTraceSlices(TracingLayer layer = TracingLayer::Count);

    void resetView();

    float &rscaling();
    QVector3D &rtranslation();

    bool saveTracingData(QString path, bool promptOnOverwrite = true);
    bool loadTracingData(QString path);

    void addPoint(QPoint newPoint, bool first);
    void erasePoint(QPoint newPoint, bool first);

    QMatrix4x4 getMVPMatrix() const;

    QMatrix4x4 getWindowToNIFTIMatrix(bool includeMVP = true) const;
    QMatrix4x4 getWindowToOpenGLMatrix(bool includeMVP = true, bool flipY = true) const;
    QMatrix4x4 getNIFTIToOpenGLMatrix(bool includeMVP = true, bool flipY = true) const;

    void setUndoStack(QUndoStack *stack);

    void setDirty(int bit);

    void updateTexture();
    void updateTrace(TracingLayer layer);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void initializeSliceView();
    void initializeTracing();
    void initializeColorMaps();

    void mouseMoveEvent(QMouseEvent *eventMove);
    void mousePressEvent(QMouseEvent *eventPress);
    void mouseReleaseEvent(QMouseEvent *eventRelease);

    void wheelEvent(QWheelEvent *event);

    void leaveEvent(QEvent *event);
};

#endif // AXIALSLICEWIDGET_H
