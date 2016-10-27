#ifndef AXIALFATSLICEWIDGET_H
#define AXIALFATSLICEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

class AxialFatSliceWidget : public QOpenGLWidget, protected QOpenGLFunctions
{

public:
    AxialFatSliceWidget(QWidget *parent);// : QOpenGLWidget(parent) {}
    ~AxialFatSliceWidget();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
};

#endif // AXIALFATSLICEWIDGET_H
