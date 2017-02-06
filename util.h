#ifndef UTIL_H
#define UTIL_H

#include <QFileInfo>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QOpenGLFunctions_3_3_Core>
#include <QKeySequence>
#include <QDebug>

// Checks if there was an OpenGL error
#define glCheckError() { GLenum err; \
    if ((err = glGetError()) != GL_NO_ERROR) \
        qDebug() << "Unable to perform OpenGL action: " << err; }

namespace util
{

/* Determines if the filename exists and is a file, returns true, otherwise returns false */
bool fileExists(QString filename);

QKeySequence getStandardSequence(QKeySequence::StandardKey standardKey, QKeySequence defaultSequence);

QVector4D clamp(QVector4D vector, QVector4D min, QVector4D max);
QVector3D clamp(QVector3D vector, QVector3D min, QVector3D max);
QVector2D clamp(QVector2D vector, QVector2D min, QVector2D max);
QPoint clamp(QPoint point, QPoint min, QPoint max);
QPoint clamp(QPoint point, QRect bounds);
QPointF clamp(QPointF point, QPointF min, QPointF max);
QPointF clamp(QPointF point, QRectF bounds);
QRect clamp(QRect rect, QPoint min, QPoint max);
QRect clamp(QRect rect, QRect bounds);
QRectF clamp(QRectF rect, QPointF min, QPointF max);
QRectF clamp(QRectF rect, QRectF bounds);

// Note: percent needs to be in range of [0.0, 1.0]
// 0.0: Return start, 1.0: Return end
QVector4D lerp(QVector4D start, QVector4D end, float percent);
QVector3D lerp(QVector3D start, QVector3D end, float percent);
QVector2D lerp(QVector2D start, QVector2D end, float percent);
QPointF lerp(QPointF start, QPointF end, float percent);
QPoint lerp(QPoint start, QPoint end, float percent);

}

#endif // UTIL_H
