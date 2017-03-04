#ifndef VERTEX_H
#define VERTEX_H

#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

/* Vertex definitions for OpenGL objects. This uses QVector*D objects provided by QT to store position, color, texture position, and normal vectors
 * in a structure for a vertex. There are multiple definitions of the Vertex structure which contain various combinations of position, color, texture
 * position, and normal vectors. The naming structure is as follows:
 * Vertex****
 *      P - Position vector (3D vector)
 *      C - Color vector
 *          4 - Specifies that the color contains an alpha component (4D vector)
 *          Default - 3D vector that contains R, G, and B component
 *      T - Texture position coordinates (2D vector)
 */

struct VertexPCT
{
    QVector3D pos;
    QVector3D color;
    QVector2D texPos;

    Q_DECL_CONSTEXPR VertexPCT() {}
    Q_DECL_CONSTEXPR explicit VertexPCT(const QVector3D &pos_) : pos(pos_) {}
    Q_DECL_CONSTEXPR VertexPCT(const QVector3D &pos_, const QVector3D &color_) : pos(pos_), color(color_) {}
    Q_DECL_CONSTEXPR VertexPCT(const QVector3D &pos_, const QVector3D &color_, const QVector2D &texPos_) : pos(pos_), color(color_), texPos(texPos_) {}

    static const int PosTupleSize = 3;
    static const int ColorTupleSize = 3;
    static const int TexPosTupleSize = 2;
    static Q_DECL_CONSTEXPR int posOffset() { return offsetof(VertexPCT, pos); }
    static Q_DECL_CONSTEXPR int colorOffset() { return offsetof(VertexPCT, color); }
    static Q_DECL_CONSTEXPR int texPosOffset() { return offsetof(VertexPCT, texPos); }
    static Q_DECL_CONSTEXPR int stride() { return sizeof(VertexPCT); }
};

struct VertexPC4T
{
    QVector3D pos;
    QVector4D color;
    QVector2D texPos;

    Q_DECL_CONSTEXPR VertexPC4T() {}
    Q_DECL_CONSTEXPR explicit VertexPC4T(const QVector3D &pos_) : pos(pos_) {}
    Q_DECL_CONSTEXPR VertexPC4T(const QVector3D &pos_, const QVector4D &color_) : pos(pos_), color(color_) {}
    Q_DECL_CONSTEXPR VertexPC4T(const QVector3D &pos_, const QVector4D &color_, const QVector2D &texPos_) : pos(pos_), color(color_), texPos(texPos_) {}

    static const int PosTupleSize = 3;
    static const int ColorTupleSize = 4;
    static const int TexPosTupleSize = 2;
    static Q_DECL_CONSTEXPR int posOffset() { return offsetof(VertexPC4T, pos); }
    static Q_DECL_CONSTEXPR int colorOffset() { return offsetof(VertexPC4T, color); }
    static Q_DECL_CONSTEXPR int texPosOffset() { return offsetof(VertexPC4T, texPos); }
    static Q_DECL_CONSTEXPR int stride() { return sizeof(VertexPC4T); }
};

struct VertexPT
{
    QVector3D pos;
    QVector2D texPos;

    Q_DECL_CONSTEXPR VertexPT() {}
    Q_DECL_CONSTEXPR explicit VertexPT(const QVector3D &pos_) : pos(pos_) {}
    Q_DECL_CONSTEXPR VertexPT(const QVector3D &pos_, const QVector2D &texPos_) : pos(pos_), texPos(texPos_) {}

    static const int PosTupleSize = 3;
    static const int TexPosTupleSize = 2;
    static Q_DECL_CONSTEXPR int posOffset() { return offsetof(VertexPT, pos); }
    static Q_DECL_CONSTEXPR int texPosOffset() { return offsetof(VertexPT, texPos); }
    static Q_DECL_CONSTEXPR int stride() { return sizeof(VertexPT); }
};

struct VertexPC
{
    QVector3D pos;
    QVector3D color;

    Q_DECL_CONSTEXPR VertexPC() {}
    Q_DECL_CONSTEXPR explicit VertexPC(const QVector3D &pos_) : pos(pos_) {}
    Q_DECL_CONSTEXPR VertexPC(const QVector3D &pos_, const QVector3D &color_) : pos(pos_), color(color_) {}

    static const int PosTupleSize = 3;
    static const int ColorTupleSize = 3;
    static Q_DECL_CONSTEXPR int posOffset() { return offsetof(VertexPC, pos); }
    static Q_DECL_CONSTEXPR int colorOffset() { return offsetof(VertexPC, color); }
    static Q_DECL_CONSTEXPR int stride() { return sizeof(VertexPC); }
};

Q_DECLARE_TYPEINFO(VertexPCT, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(VertexPC4T, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(VertexPT, Q_MOVABLE_TYPE);
Q_DECLARE_TYPEINFO(VertexPC, Q_MOVABLE_TYPE);

#endif // VERTEX_H
