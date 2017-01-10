#ifndef UTIL_H
#define UTIL_H

#include <QFileInfo>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

namespace util
{

/* Determines if the filename exists and is a file, returns true, otherwise returns false */
bool fileExists(QString filename);

QVector4D clamp(QVector4D vector, QVector4D min, QVector4D max);
QVector3D clamp(QVector3D vector, QVector3D min, QVector3D max);
QVector2D clamp(QVector2D vector, QVector2D min, QVector2D max);

}

#endif // UTIL_H
