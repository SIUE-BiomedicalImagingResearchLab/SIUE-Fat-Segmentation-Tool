#include "util.h"

namespace util
{

bool fileExists(QString filename)
{
    QFileInfo fileInfo(filename);
    return (fileInfo.exists() && fileInfo.isFile());
}

QVector4D clamp(QVector4D vector, QVector4D min, QVector4D max)
{
    vector.setX(std::min(std::max(vector.x(), min.x()), max.x()));
    vector.setY(std::min(std::max(vector.y(), min.y()), max.y()));
    vector.setZ(std::min(std::max(vector.z(), min.z()), max.z()));
    vector.setW(std::min(std::max(vector.w(), min.w()), max.w()));

    return vector;
}

QVector3D clamp(QVector3D vector, QVector3D min, QVector3D max)
{
    vector.setX(std::min(std::max(vector.x(), min.x()), max.x()));
    vector.setY(std::min(std::max(vector.y(), min.y()), max.y()));
    vector.setZ(std::min(std::max(vector.z(), min.z()), max.z()));

    return vector;
}

QVector2D clamp(QVector2D vector, QVector2D min, QVector2D max)
{
    vector.setX(std::min(std::max(vector.x(), min.x()), max.x()));
    vector.setY(std::min(std::max(vector.y(), min.y()), max.y()));

    return vector;
}

QVector4D lerp(QVector4D start, QVector4D end, float percent)
{
    return (start + percent * (end - start));
}

QVector3D lerp(QVector3D start, QVector3D end, float percent)
{
    return (start + percent * (end - start));
}

QVector2D lerp(QVector2D start, QVector2D end, float percent)
{
    return (start + percent * (end - start));
}

QPointF lerp(QPointF start, QPointF end, float percent)
{
    return (start + percent * (end - start));
}

QPoint lerp(QPoint start, QPoint end, float percent)
{
    const QPointF start_(start);
    const QPointF end_(end);

    return lerp(start_, end_, percent).toPoint();
}

}
