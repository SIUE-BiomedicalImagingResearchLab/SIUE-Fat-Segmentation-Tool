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

}
