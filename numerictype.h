#ifndef NUMERICTYPE_H
#define NUMERICTYPE_H

#include <QDebug>

enum class DataType : int
{
    None = -1,
    Bool = 0,
    UnsignedChar,
    Char,
    UnsignedShort,
    Short,
    UnsignedInt,
    Int,
    UnsignedLong,
    Long,
    Float,
    Double,
    Count
};

#ifndef NUMERIC_TYPE_NO_OPENGL
#include <QOpenGLFunctions_3_3_Core>
#endif // NUMERIC_TYPE_NO_OPENGL

#ifndef NUMERIC_TYPE_NO_OPENCV
#include <opencv2/opencv.hpp>
#endif // NUMERIC_TYPE_NO_OPENCV

#ifndef NUMERIC_TYPE_NO_NIFTI
#include <nifti1.h>
#include <nifti1_io.h>
#endif // NUMERIC_TYPE_NO_NIFTI

struct NumericType
{
    NumericType(DataType type_, GLenum openGLFormat_, GLenum openGLType_, int openCVType_, int openCVTypeNoChannel_, int NIFTIType_);

    const DataType type;

    double getMin() const;
    double getMax() const;

#ifndef NUMERIC_TYPE_NO_OPENGL
    const GLenum openGLFormat;
    const GLenum openGLType;

    static const NumericType *OpenGL(GLenum format, GLenum type);
#endif // NUMERIC_TYPE_NO_OPENGL

#ifndef NUMERIC_TYPE_NO_OPENCV
    const int openCVType;
    const int openCVTypeNoChannel;

    static const NumericType *OpenCV(int type);
    static const NumericType *OpenCV(int type, int numChannels);
#endif // NUMERIC_TYPE_NO_OPENCV

#ifndef NUMERIC_TYPE_NO_NIFTI
    const int NIFTIType;

    static const NumericType *NIFTI(int type);
#endif // NUMERIC_TYPE_NO_NIFTI
};

#endif // NUMERICTYPE_H
