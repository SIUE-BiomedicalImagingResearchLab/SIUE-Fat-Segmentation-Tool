#include "util.h"

namespace util
{

bool fileExists(QString filename)
{
    QFileInfo fileInfo(filename);
    return (fileInfo.exists() && fileInfo.isFile());
}

QKeySequence getStandardSequence(QKeySequence::StandardKey standardKey, QKeySequence defaultSequence)
{
    QKeySequence ret(standardKey);

    if (ret.isEmpty())
        return defaultSequence;

    return ret;
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

QPoint clamp(QPoint point, QPoint min, QPoint max)
{
    point.setX(std::min(std::max(point.x(), min.x()), max.x()));
    point.setY(std::min(std::max(point.y(), min.y()), max.y()));

    return point;
}

QPoint clamp(QPoint point, QRect bounds)
{
    return clamp(point, bounds.topLeft(), bounds.bottomRight());
}

QPointF clamp(QPointF point, QPointF min, QPointF max)
{
    point.setX(std::min(std::max(point.x(), min.x()), max.x()));
    point.setY(std::min(std::max(point.y(), min.y()), max.y()));

    return point;
}

QPointF clamp(QPointF point, QRectF bounds)
{
    return clamp(point, bounds.topLeft(), bounds.bottomRight());
}

QRect clamp(QRect rect, QPoint min, QPoint max)
{
    rect.setTopLeft(clamp(rect.topLeft(), min, max));
    rect.setBottomRight(clamp(rect.bottomRight(), min, max));

    return rect;
}

QRect clamp(QRect rect, QRect bounds)
{
    rect.setTopLeft(clamp(rect.topLeft(), bounds));
    rect.setBottomRight(clamp(rect.bottomRight(), bounds));

    return rect;
}

QRectF clamp(QRectF rect, QPointF min, QPointF max)
{
    rect.setTopLeft(clamp(rect.topLeft(), min, max));
    rect.setBottomRight(clamp(rect.bottomRight(), min, max));

    return rect;
}

QRectF clamp(QRectF rect, QRectF bounds)
{
    rect.setTopLeft(clamp(rect.topLeft(), bounds));
    rect.setBottomRight(clamp(rect.bottomRight(), bounds));

    return rect;
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

QString execCommand(const char *cmd)
{
    char buffer[128];
    QString result;
#ifdef Q_OS_WIN
    FILE *pipe = _popen(cmd, "r");
#else // Q_OS_WIN
    FILE *pipe = popen(cmd, "r");
#endif // Q_OS_WIN

    if (!pipe)
        return QString();

    while (!feof(pipe))
    {
        if (fgets(&buffer[0], 128, pipe) != NULL)
            result += &buffer[0];
    }

#ifdef Q_OS_WIN
    _pclose(pipe);
#else // Q_OS_WIN
    pclose(pipe);
#endif // Q_OS_WIN
    return result;
}

QString execCommand(QString cmd)
{
    return execCommand(cmd.toStdString().c_str());
}

}
