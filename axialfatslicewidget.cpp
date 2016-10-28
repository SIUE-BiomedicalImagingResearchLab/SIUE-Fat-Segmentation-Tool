#include "axialfatslicewidget.h"

AxialFatSliceWidget::AxialFatSliceWidget(QWidget *parent) : QOpenGLWidget(parent)
{

}

void AxialFatSliceWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
}

void AxialFatSliceWidget::resizeGL(int w, int h)
{
    // Update projection matrix and other size related settings:
    //m_projection.setToIdentity();
    //m_projection.perspective(45.0f, w / float(h), 0.01f, 100.0f);
}

void AxialFatSliceWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void AxialFatSliceWidget::mouseMoveEvent(QMouseEvent *eventMove)
{
    //qDebug() << "MOVE!";
}

void AxialFatSliceWidget::mousePressEvent(QMouseEvent *eventPress)
{
    //qDebug() << "PRESS";
}

void AxialFatSliceWidget::mouseReleaseEvent(QMouseEvent *eventRelease)
{
    //qDebug() << "RELEASE";
}

AxialFatSliceWidget::~AxialFatSliceWidget()
{

}
