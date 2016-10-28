#ifndef AXIALFATSLICEWIDGET_H
#define AXIALFATSLICEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QDebug>
#include <QMouseEvent>

class AxialFatSliceWidget : public QOpenGLWidget, protected QOpenGLFunctions
{

    // Should I include the NIFTI file here? Should this just be a basic editor maybe?

public:
    AxialFatSliceWidget(QWidget *parent);// : QOpenGLWidget(parent) {}
    ~AxialFatSliceWidget();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void mouseMoveEvent(QMouseEvent *eventMove);
    void mousePressEvent(QMouseEvent *eventPress);
    void mouseReleaseEvent(QMouseEvent *eventRelease);
};

#endif // AXIALFATSLICEWIDGET_H
