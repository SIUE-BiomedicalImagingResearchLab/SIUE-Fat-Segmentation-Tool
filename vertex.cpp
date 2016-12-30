#include "vertex.h"

// VertexPCT
// --------------------------------------------------------------------------------------------------------------------
Q_DECLARE_TYPEINFO(VertexPCT, Q_MOVABLE_TYPE);
Q_DECL_CONSTEXPR VertexPCT::VertexPCT() {}
Q_DECL_CONSTEXPR VertexPCT::VertexPCT(const QVector3D &pos_) : pos(pos_) {}
Q_DECL_CONSTEXPR VertexPCT::VertexPCT(const QVector3D &pos_, const QVector3D &color_) : pos(pos_), color(color_) {}
Q_DECL_CONSTEXPR VertexPCT::VertexPCT(const QVector3D &pos_, const QVector3D &color_, const QVector2D &texPos_) : pos(pos_), color(color_), texPos(texPos_) {}

Q_DECL_CONSTEXPR int VertexPCT::posOffset() { return offsetof(VertexPCT, pos); }
Q_DECL_CONSTEXPR int VertexPCT::colorOffset()  { return offsetof(VertexPCT, color); }
Q_DECL_CONSTEXPR int VertexPCT::texPosOffset()  { return offsetof(VertexPCT, texPos); }
Q_DECL_CONSTEXPR int VertexPCT::stride()  { return sizeof(VertexPCT); }

// VertexPC4T
// --------------------------------------------------------------------------------------------------------------------
Q_DECLARE_TYPEINFO(VertexPC4T, Q_MOVABLE_TYPE);
Q_DECL_CONSTEXPR VertexPC4T::VertexPC4T() {}
Q_DECL_CONSTEXPR VertexPC4T::VertexPC4T(const QVector3D &pos_) : pos(pos_) {}
Q_DECL_CONSTEXPR VertexPC4T::VertexPC4T(const QVector3D &pos_, const QVector4D &color_) : pos(pos_), color(color_) {}
Q_DECL_CONSTEXPR VertexPC4T::VertexPC4T(const QVector3D &pos_, const QVector4D &color_, const QVector2D &texPos_) : pos(pos_), color(color_), texPos(texPos_) {}

Q_DECL_CONSTEXPR int VertexPC4T::posOffset() { return offsetof(VertexPC4T, pos); }
Q_DECL_CONSTEXPR int VertexPC4T::colorOffset()  { return offsetof(VertexPC4T, color); }
Q_DECL_CONSTEXPR int VertexPC4T::texPosOffset()  { return offsetof(VertexPC4T, texPos); }
Q_DECL_CONSTEXPR int VertexPC4T::stride()  { return sizeof(VertexPC4T); }

// VertexPT
// --------------------------------------------------------------------------------------------------------------------
Q_DECLARE_TYPEINFO(VertexPT, Q_MOVABLE_TYPE);
Q_DECL_CONSTEXPR VertexPT::VertexPT() {}
Q_DECL_CONSTEXPR VertexPT::VertexPT(const QVector3D &pos_) : pos(pos_) {}
Q_DECL_CONSTEXPR VertexPT::VertexPT(const QVector3D &pos_, const QVector2D &texPos_) : pos(pos_), texPos(texPos_) {}

Q_DECL_CONSTEXPR int VertexPT::posOffset() { return offsetof(VertexPT, pos); }
Q_DECL_CONSTEXPR int VertexPT::texPosOffset()  { return offsetof(VertexPT, texPos); }
Q_DECL_CONSTEXPR int VertexPT::stride()  { return sizeof(VertexPT); }

// VertexPC
// --------------------------------------------------------------------------------------------------------------------
Q_DECLARE_TYPEINFO(VertexPC, Q_MOVABLE_TYPE);
Q_DECL_CONSTEXPR VertexPC::VertexPC() {}
Q_DECL_CONSTEXPR VertexPC::VertexPC(const QVector3D &pos_) : pos(pos_) {}
Q_DECL_CONSTEXPR VertexPC::VertexPC(const QVector3D &pos_, const QVector3D &color_) : pos(pos_), color(color_) {}

Q_DECL_CONSTEXPR int VertexPC::posOffset() { return offsetof(VertexPC, pos); }
Q_DECL_CONSTEXPR int VertexPC::colorOffset()  { return offsetof(VertexPC, color); }
Q_DECL_CONSTEXPR int VertexPC::stride()  { return sizeof(VertexPC); }
